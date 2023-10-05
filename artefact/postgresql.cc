/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            postgresql.cc
 *
 *  Fri Feb 16 13:49:53 CET 2007
 *  Copyright  2006 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Artefact.
 *
 *  Artefact is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Artefact is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Artefact; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "postgresql.h"

#include <hugin.hpp>

#include <exception>
#include <iostream>
#include <string>

#include "uid.h"

#include <stdlib.h>

class PGresultAutoFree {
public:
  PGresultAutoFree(PGresult *pg) { this->pg = pg; }
  ~PGresultAutoFree() { PQclear(pg); }

private:
  PGresult *pg;
};

PostgreSQL::PostgreSQL(std::string host, 
											 unsigned short port, 
											 std::string user, 
											 std::string password, 
											 std::string database)
{
  DEBUG(db, "Host: %s\n", host.c_str());
  DEBUG(db, "Port: %d\n", port);
  DEBUG(db, "User: %s\n", user.c_str());
  DEBUG(db, "Password: %s\n", password.c_str());
  DEBUG(db, "Database: %s\n", database.c_str());

  this->host = host;
  this->port = port;
  this->user = user;
  this->password = password;
  this->database = database;

  c = NULL;

  try {
    openDb();
  } catch(...) {
    // Ignore errors here. Database will try to reconnect later.
  }
}

PostgreSQL::~PostgreSQL()
{
#ifndef WITHOUT_DB
  PQfinish(c);
#endif/*WITHOUT_DB*/
}

void PostgreSQL::openDb()
{
#ifndef WITHOUT_DB

  PGresult *pg = PQexec(c, "SELECT version();"); // Send ping?
  PGresultAutoFree f(pg);

  if(PQresultStatus(pg) == PGRES_TUPLES_OK) return; // Connection ok.

  PQfinish(c); // Bad connection, close it, so we can open a new one.

  std::string pwd;
  if(password != "") {
    pwd = " password=";
    pwd += password;
  }

  char port_string[32];
  sprintf(port_string, "%d", port);
  std::string hoststring = "host=" + host + " port=" + port_string +
    " user=" + user + pwd + " dbname=" + database;
 
  INFO(db, "Connecting to database: %s:%d", host.c_str(), port);

  c =  PQconnectdb(hoststring.c_str());

  if(c == NULL || PQstatus(c) == CONNECTION_BAD)	{
    ERR(db, "Could not connect to database");
    throw PostgreSQLException("Database connection error.");
  }
#endif/*WITHOUT_DB*/
}

PostgreSQLTransaction *PostgreSQL::transaction(bool pretend)
{
  try {
    openDb(); // Make sure we have a working database connection.
  } catch(...) {
    return NULL;
  }

  DEBUG(db, "New transaction, pretend == %s\n", pretend?"true":"false");
  return new PostgreSQLTransaction(pretend, c);
}

PostgreSQLTransaction::PostgreSQLTransaction(bool pretend, PGconn *conn)
  : c(conn)
{
  this->pretend = pretend;
  std::string s = "BEGIN;";
  PGresult *pg = PQexec(c, s.c_str());
  PGresultAutoFree f(pg);

  if(PQresultStatus(pg) != PGRES_COMMAND_OK) {
    ERR(db, "Failed to begin transaction (%s)\n", s.c_str());
  }
}

PostgreSQLTransaction::~PostgreSQLTransaction()
{
  std::string s;
  if(pretend) {
    DEBUG(db, "abort\n");
    s = "ROLLBACK;";
  } else {
    DEBUG(db, "commit\n");
    s = "COMMIT;";
  }
  PGresult *pg = PQexec(c, s.c_str());
  PGresultAutoFree f(pg);

  if(PQresultStatus(pg) != PGRES_COMMAND_OK) {
    ERR(db, "Failed to end transaction (%s)\n", s.c_str());
  }
}

/*
 * DB layout:
 *
 * patient table: (CPR,
 *                 data_uid)
 *
 * data table: (data_uid,
 *              device_type,
 *              device_id,
 *              classname,
 *              timestamp,
 *              location,
 *              filename,
 *              file_length)
 */
void PostgreSQLTransaction::post(Pentominos::Entry &entry,
                                 Pentominos::Data &data)
{
#ifndef WITHOUT_DB
  std::string uid = Pentominos::getUID();
    
  std::vector< std::string >::iterator j = data.classlist.begin();
  while(j != data.classlist.end()) {
      
    std::string classname = *j;
      
    char lenbuf[256];
    sprintf(lenbuf, "%d", data.length);
    std::string file_length = lenbuf;
      
    char stimestamp[32];
    sprintf(stimestamp, "%u", (unsigned int)data.timestamp);
      
    // Insert data.
    std::string transaction = "INSERT INTO data VALUES('" + 
      uid + "', '" +
      data.device_type + "', '" +
      data.device_id + "', '" +
      classname + "', '" +
      stimestamp + "', '" +
      data.location + "', '" +
      data.filename + "', '" +
      file_length + "')";
    DEBUG(db, "%s\n", transaction.c_str());
    PGresult *pg = PQexec(c, transaction.c_str());
    PGresultAutoFree f(pg);

    if(PQresultStatus(pg) != PGRES_COMMAND_OK) {
      throw PostgreSQLException("Failed to insert patient");
    }

    j++;
  }
    
  {
    // Insert data reference into cpr record.
    std::string transaction = "INSERT INTO patient VALUES('" + 
      entry.patientid + "', '" +
      data.operatorid + "', '" +
      uid + "')";
    DEBUG(db, "%s\n", transaction.c_str());
    
    PGresult *pg = PQexec(c, transaction.c_str());
    PGresultAutoFree f(pg);

    if(PQresultStatus(pg) != PGRES_COMMAND_OK) {
      throw PostgreSQLException("Failed to insert patient");
    }
  }
#endif/*WITHOUT_DB*/
}

static std::list<std::vector<std::string> > toArray(PGresult *pg)
{
  std::list<std::vector<std::string> > res;
  for(int j = 0; j < PQntuples(pg); j++) {
    std::vector<std::string> v;
    for(int i = 0; i < PQnfields(pg); i++) {
      v.push_back(std::string(PQgetvalue(pg, j, i)));
    }
    res.push_back(v);
  }
  return res;
}

std::list< Pentominos::Data >
PostgreSQLTransaction::get(std::string cpr, std::string cls,
                           bool use_from, time_t from, bool use_to, time_t to)
{
	std::list< Pentominos::Data > entries;

#ifndef WITHOUT_DB
  try {
    std::string query;

    std::string timeq;

    char ts_from[64];
    snprintf(ts_from, sizeof(ts_from), "%lu", from);

    char ts_to[64];
    snprintf(ts_to, sizeof(ts_to), "%lu", to);

    if(use_from)
      timeq += std::string(" AND CAST(timestamp as int)>=") + ts_from;
    if(use_to) 
      timeq += std::string(" AND CAST(timestamp as int)<=") + ts_to;

    query = "SELECT"
      " d.data_uid,"
      " d.device_type,"
      " d.device_id,"
      " d.class,"
      " d.timestamp,"
      " d.location,"
      " d.filename,"
      " d.file_length FROM data d, patient p WHERE"
      " d.data_uid = p.uid AND p.cpr='"+cpr+"'"
      " AND d.class='" + cls + "' " + timeq;

    DEBUG(db, "%s\n", query.c_str());
    PGresult *pg = PQexec(c, query.c_str());
    PGresultAutoFree f(pg);

    if (PQresultStatus(pg) != PGRES_TUPLES_OK) {
      ERR(db, "No tuples...\n");
      return entries;
    }

    std::list<std::vector<std::string> > D = toArray(pg);

    std::list<std::vector<std::string> >::iterator di = D.begin();
    while(di != D.end())	{
      Pentominos::Data data;

      std::vector<std::string> &d = *di;

      data.uid = d[0].c_str();
      data.device_type = d[1].c_str();
      data.device_id = d[2].c_str();
      data.classlist.push_back(d[3].c_str());
      data.timestamp = atoll(d[4].c_str());
      data.location = d[5].c_str();
      data.filename = d[6].c_str();
      data.length = atoll((char*)d[7].c_str());
      
      entries.push_back(data);
      
      di++;
    }

  }	catch(const std::exception &e) {
		throw PostgreSQLException(e.what());
	}
#endif/*WITHOUT_DB*/

  return entries;
}


/*

CREATE DATABASE artefact
  WITH OWNER = artefact
       ENCODING = 'UNICODE'
       TABLESPACE = pg_default;

CREATE TABLE patient
(
  cpr varchar(255),
  uid varchar(255)
) 
WITH OIDS;
ALTER TABLE patient OWNER TO artefact;

CREATE TABLE data
(
  data_uid varchar(255),
  device_type varchar(255),
  device_id varchar(255),
  "timestamp" varchar(255),
  "location" varchar(255),
  filename varchar(255),
  file_length varchar(255)
) 
WITH OIDS;
ALTER TABLE data OWNER TO artefact;

*/

#ifdef TEST_POSTGRESQL
//deps: ../lib/uid.cc ../lib/exception.cc ../lib/debug.cc ../lib/log.cc ../lib/entry.cc ../lib/utf8.cc
//cflags: $(PQXX_CXXFLAGS) -I../lib -I..
//libs: $(PQXX_LIBS)
#include "test.h"

#define CPR "0000000000"

TEST_BEGIN;

///////
/////// Test 'pretend' feature in database.
///////
size_t num;

PostgreSQL p("localhost", 5432, "artefact", "artefact", "artefact");
PostgreSQLTransaction *t = p.transaction(true);

{ // get initial data count.
  std::list< Pentominos::Data > res = t->get(CPR, "dims");
  num = res.size();
}

Pentominos::Entry entry;
entry.patientid = CPR;

Pentominos::Data data;

data.classlist.push_back("dims");
data.classlist.push_back("dut");

data.length = 10;
data.timestamp = time(NULL);
data.device_type = "abekat";
data.device_id = "hun";
data.location = "abeburet";
data.filename = "/tmp/guldkaramel";
data.operatorid = "1234";

t->post(entry, data);

std::list< Pentominos::Data > res = t->get(CPR, "dims");
std::list< Pentominos::Data >::iterator i = res.begin();
TEST_EQUAL_INT(res.size(), num + 1, "Did we get our own data?");
while(i != res.end()) {
  i++;
}

delete t;

{
  PostgreSQLTransaction *t = p.transaction(true);

  std::list< Pentominos::Data > res = t->get(CPR, "dims");
  std::list< Pentominos::Data >::iterator i = res.begin();
  TEST_EQUAL_INT(res.size(), num, "We should not get last data?");
  while(i != res.end()) {
    i++;
  }

  delete t;
}

TEST_END;

#endif/*TEST_POSTGRESQL*/
