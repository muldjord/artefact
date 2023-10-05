/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luapostgresql.cc
 *
 *  Mon Sep  1 09:30:52 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Pentominos.
 *
 *  Pentominos is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Pentominos is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Pentominos; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "luapostgresql.h"

#include <hugin.hpp>

#include <luautil.h>

#include <vector>
#include <libpq-fe.h>

#ifndef TEST_TEST_LUA
#include <sysconfig.h>
#endif

static std::vector< PGconn * > connections;

/*
static int _connect(lua_State *L)
{
  int n = lua_gettop(L); // number of arguments
  if(n != 1) {
    char errstr[512];
    sprintf(errstr, "Number of args expected 1, got %d", n);
    fprintf(stderr, errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }
  
  size_t size;
  const char *str = lua_tolstring(L, lua_gettop(L), &size);

  try {
    LUADataParser *parser = (LUADataParser*)Pentominos::getGlobal(L, global_parser);
    parser->socket->write((char*)str, size);
    parser->md5sum.update((char*)str, size);
  } catch( Pentominos::Exception &e ) {
    fprintf(stderr, "%s\n", e.what());
  }

  return 0;
}

int getCPR(lua_State *L)
{
  int n = lua_gettop(L); // number of arguments
  if(n != 0) {
    char errstr[512];
    sprintf(errstr, "Number of args expected 0, got %d", n);
    fprintf(stderr, errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }
  
  LUADataParser *parser = (LUADataParser*)Pentominos::getGlobal(L, global_parser);
  lua_pushstring(L, parser->entry->cpr.c_str());
  return 1;
}
*/

static int pg_connect(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_STRING,
                              Pentominos::T_STRING,
                              Pentominos::T_STRING,
                              Pentominos::T_STRING,
                              Pentominos::T_END);

  std::string db = lua_tostring(L, lua_gettop(L));
  std::string pwd = lua_tostring(L, lua_gettop(L) - 1);
  std::string user = lua_tostring(L, lua_gettop(L) - 2);
  std::string port = lua_tostring(L, lua_gettop(L) - 3);
  std::string host = lua_tostring(L, lua_gettop(L) - 4);

  DEBUG(luapostgres, "db [%s]\n", db.c_str());
  DEBUG(luapostgres, "pwd [%s]\n", pwd.c_str());
  DEBUG(luapostgres, "user [%s]\n", user.c_str());
  DEBUG(luapostgres, "host [%s]\n", host.c_str());
  DEBUG(luapostgres, "port [%s]\n", port.c_str());

#ifndef WITHOUT_DB
  std::string hoststring = "host=" + host + " port=" + port 
    + " dbname=" + db + " user=" + user;
  if(pwd != "") hoststring += " password=" + pwd;

  PGconn *c = PQconnectdb(hoststring.c_str());

  if(c == NULL)	return 0;

  connections.push_back(c);

  lua_pushinteger(L, connections.size() - 1);

  return 1;
#else
  return 0;
#endif/*WITHOUT_DB*/
}

static int pg_close(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_END);
  
#ifndef WITHOUT_DB
  unsigned int pg = lua_tointeger(L, lua_gettop(L));

  if(connections.size() <= pg || connections[pg] == NULL) {
    ERR(luapostgres, "missing connection\n");
    return 0;
  }

  PQfinish(connections[pg]);
  connections[pg] = NULL;
#endif/* WITHOUT_DB*/

  return 0;
}

static int pg_query(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
  
  unsigned int pgn = lua_tointeger(L, lua_gettop(L) - 1);
  std::string query = lua_tostring(L, lua_gettop(L));

  DEBUG(luapostgres, "query [%s]\n", query.c_str());

#ifndef WITHOUT_DB
  if(connections.size() <= pgn || connections[pgn] == NULL) {
    ERR(luapostgres, "missing connection\n");
    return 0;
  }

  PGconn *c = connections[pgn];

  PGresult *pg = PQexec(c, query.c_str());

  if(PQresultStatus(pg) != PGRES_TUPLES_OK) {
    PQclear(pg);
    ERR(luapostgres, "query error [%s]\n", PQresultErrorMessage(pg));
    return 0;
  }

  int tuples = PQntuples(pg);
  for(int j = 0; j < tuples; j++) {

    DEBUG(luapostgres, "%d columns in answer:\n", tuples);
    int fields = PQnfields(pg);
    lua_createtable(L, 0, fields);
    int top = lua_gettop(L);

    for(int i = 0; i < (int)  fields; i++) {
      std::string field = PQgetvalue(pg, j, i);
      DEBUG(luapostgres, "\t[%s]\n", field.c_str());
      lua_pushstring(L, field.c_str());
      lua_rawseti(L, top, i);
    }

  }
#else
  (void)pg;
#endif/*WITHOUT_DB*/

  return 1;
}

void register_postgresql(lua_State *L)
{
  lua_register(L, "pg_connect", pg_connect);
  lua_register(L, "pg_close", pg_close);
  lua_register(L, "pg_query", pg_query);
}
