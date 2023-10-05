/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            postgresql.h
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
#ifndef __ARTEFACT_POSTGRESQL_H__
#define __ARTEFACT_POSTGRESQL_H__

#include <sysconfig.h>

#include <string>
#include <list>
#include <libpq-fe.h>

#include "entry.h"
#include "tree.h"
#include "exception.h"

/**
 * PostgreSQLException.
 */
class PostgreSQLException : public Pentominos::Exception {
public:
	PostgreSQLException(std::string reason)
    : Pentominos::Exception("PostGreSQL error: " + reason) {}
};

class PostgreSQLTransaction {
public:
  PostgreSQLTransaction(bool pretend, PGconn *c);
  ~PostgreSQLTransaction();

	void post(Pentominos::Entry &entry, Pentominos::Data &data);
  std::list< Pentominos::Data > get(std::string cpr, std::string cls,
                                    bool use_from = false, time_t from = 0,
                                    bool use_to = false, time_t to = 0);
#ifndef WITHOUT_DB
  PGconn *c;
#endif/*WITHOUT_DB*/
  bool pretend;
};

class PostgreSQL {
public:
	PostgreSQL(std::string host, 
             unsigned short port, 
             std::string user, 
             std::string password, 
             std::string database);
	~PostgreSQL();

  PostgreSQLTransaction *transaction(bool pretend);

private:
#ifndef WITHOUT_DB
  PGconn *c;
#endif/*WITHOUT_DB*/

  void openDb();

  std::string host;
  unsigned short port;
  std::string user;
  std::string password;
  std::string database;
};

#endif/*__ARTEFACT_POSTGRESQL_H__*/
