/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luautil.cc
 *
 *  Fri Apr 13 14:38:53 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
#include "luautil.h"

#include <hugin.hpp>

#include <string>

#include <map>

std::map<unsigned int, void *> pointers;

// for ntohl and htonl
#include <arpa/inet.h>

#define GLOBAL_PREFIX "magic_global_"

//
// Get the (somewhat hacky) global pointer to the parser object.
// It is set in the preload method.
//
void *Pentominos::getGlobal(lua_State *L, std::string name)
{
  unsigned int top;
  unsigned int index;

  std::string var = std::string(GLOBAL_PREFIX) + name;

  lua_getfield(L, LUA_GLOBALSINDEX, var.c_str()); 
  top = lua_gettop(L);
  index = lua_tointeger(L, top);

  return pointers.at(index);
}

void Pentominos::removeGlobal(lua_State *L, std::string name)
{
  unsigned int top;
  unsigned int index;

  std::string var = std::string(GLOBAL_PREFIX) + name;

  lua_getfield(L, LUA_GLOBALSINDEX, var.c_str()); 
  top = lua_gettop(L);
  index = lua_tointeger(L, top);

  pointers.erase(index);
}

void Pentominos::setGlobal(lua_State *L, std::string name, void *p)
{
  // Put the value of this in the globals
  char value_of_this[256];

  std::string val = std::string(GLOBAL_PREFIX) + name;

  /*
  unsigned int parser = (unsigned int)p;
  parser = htonl(parser);
  */

  unsigned int index = pointers.size();
  pointers[index] = p;

  sprintf(value_of_this, "%s = %u\n", val.c_str(), index);
  int s = luaL_loadstring(L, value_of_this);
  switch(s) {
  case 0: //no errors;
    break;
  case LUA_ERRSYNTAX: //syntax error during pre-compilation;
  case LUA_ERRMEM: //memory allocation error.
    fprintf(stderr, "Error: %s\n", lua_tostring(L, lua_gettop(L)));
  default:
    fprintf(stderr, "Unknown return value of luaL_loadstring.\n");
  }

  // Run program (init)
  s = lua_pcall(L, 0, LUA_MULTRET, 0);
  // Check for errors
  switch(s) {
  case 0: // Success
    break;
  case LUA_ERRRUN:// a runtime error.
  case LUA_ERRMEM:// memory allocation error.
    // For such errors, Lua does not call the error handler function.
  case LUA_ERRERR:// error while running the error handler function.
    fprintf(stderr, "Error: %s\n", lua_tostring(L, lua_gettop(L)));
    break;
  default:
    fprintf(stderr, "Error: Unknown return value of lua_pcall.\n");
    break;
  }
}

void Pentominos::call(lua_State *L, std::string function, int numargs)
{
  // Get function
  lua_getglobal(L, function.c_str());

  // Call it
  int s = lua_pcall(L, numargs, LUA_MULTRET, 0);

  // Check for errors
  switch(s) {
  case 0: // Success
    break;
  case LUA_ERRRUN:// a runtime error.
  case LUA_ERRMEM:// memory allocation error.
    // For such errors, Lua does not call the error handler function.
  case LUA_ERRERR:// error while running the error handler function.
    fprintf(stderr, "Error: %s\n", lua_tostring(L, lua_gettop(L)));
    break;
  default:
    fprintf(stderr, "Error: Unknown return value of lua_pcall.\n");
    break;
  }
}


/*
lua_isboolean
lua_iscfunction
lua_isfunction
lua_islightuserdata
lua_isnil
lua_isnone
lua_isnoneornil
lua_isnumber
lua_isstring
lua_istable
lua_isthread
lua_isuserdata
*/
int Pentominos::checkParameters(lua_State *L, ...)
{
  va_list ap;
  va_start(ap, L);

  size_t nargs = lua_gettop(L); // number of arguments

  size_t size = 0;
  int t;
  while(1) { 
    t = va_arg(ap, int);
    if(t == T_END) break;

    switch(t) {
    case T_STRING:
    case T_NUMBER:
    case T_BOOLEAN:
      break;

    default:
      return luaL_error(L,"Unknown type specifier [%d] at position %d. "
                        "Missing TYPE_END?", t, size+1);
    }

    size++;
  }

  va_end(ap);

  if(nargs != size) {
    return luaL_error(L, "Number of args expected %d, got %d", size, nargs);
  }

  va_start(ap, L);

  size_t idx = 0;
  while(1) {
    t = va_arg(ap, int);
    if(t == T_END) break;

    switch(t) {
    case T_STRING:
      if(lua_isstring(L, lua_gettop(L)-(size-idx-1)) == 0) {
        va_end(ap);
        return luaL_error(L, "Parameter %d should be of type string.", idx+1);
      }
      break;

    case T_NUMBER:
      if(lua_isnumber(L, lua_gettop(L)-(size-idx-1)) == 0) {
        va_end(ap);
        return luaL_error(L, "Parameter %d should be of type number.", idx+1);
      }
      break;

    case T_BOOLEAN:
      if(lua_isboolean(L, lua_gettop(L)-(size-idx-1)) == 0) {
        va_end(ap);
        return luaL_error(L, "Parameter %d should be of type boolean.", idx+1);
      }
      break;

    default:
      va_end(ap);
      return luaL_error(L,"Unknown type specifier [%d] at position %d. "
                        "Missing TYPE_END?", t, idx+1);
    }

    idx++;
  }

  va_end(ap);

  return 0;
}

#ifdef TEST_LUAUTIL
//deps:
//cflags: $(LUA_CFLAGS) -I..
//libs: $(LUA_LIBS)
#include "test.h"

#define LUAPROG \
  "testfunc('a string', 42, true)"

#define LUAPROG_BAD1 \
  "testfunc('a string', 42)"

#define LUAPROG_BAD2 \
  "testfunc('a string', 42, true, 'another one')"

#define LUAPROG_BAD3 \
  "testfunc(false, 42, 'string')"

#define LUAPROG_MISSING_END \
  "testfunc_bad('a string', 42, true, 1)"

static int testfunc(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_NUMBER,
                              Pentominos::T_BOOLEAN,
                              Pentominos::T_END);
  return 0;
}

static int testfunc_bad(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_NUMBER,
                              Pentominos::T_BOOLEAN,
                              Pentominos::T_NUMBER);
  return 0;
}


TEST_BEGIN;

int a, b, c;

lua_State *L;
L = luaL_newstate();
if(L == NULL) TEST_FATAL("Could not allocate lua state.");
luaL_openlibs(L);

Pentominos::setGlobal(L, "a", &a);
Pentominos::setGlobal(L, "b", &b);
Pentominos::setGlobal(L, "c", &c);

TEST_EQUAL(Pentominos::getGlobal(L, "b"), &b, "Test get global");
TEST_EQUAL(Pentominos::getGlobal(L, "c"), &c, "Test get global");
TEST_EQUAL(Pentominos::getGlobal(L, "a"), &a, "Test get global");

lua_register(L, "testfunc", testfunc);

if(luaL_loadstring(L, LUAPROG)) {
  TEST_FATAL(lua_tostring(L, lua_gettop(L)))
}
TEST_EQUAL_INT(lua_pcall(L, 0, LUA_MULTRET, 0), 0, "Good one");

if(luaL_loadstring(L, LUAPROG_BAD1)) {
  TEST_FATAL(lua_tostring(L, lua_gettop(L)))
}
TEST_NOTEQUAL_INT(lua_pcall(L, 0, LUA_MULTRET, 0), 0, "Too short one");

if(luaL_loadstring(L, LUAPROG_BAD2)) {
  TEST_FATAL(lua_tostring(L, lua_gettop(L)))
}
TEST_NOTEQUAL_INT(lua_pcall(L, 0, LUA_MULTRET, 0), 0, "Too long one");

if(luaL_loadstring(L, LUAPROG_BAD3)) {
  TEST_FATAL(lua_tostring(L, lua_gettop(L)))
}
TEST_NOTEQUAL_INT(lua_pcall(L, 0, LUA_MULTRET, 0), 0, "Plain wrong");


lua_register(L, "testfunc_bad", testfunc_bad);
if(luaL_loadstring(L, LUAPROG_MISSING_END)) {
  TEST_FATAL(lua_tostring(L, lua_gettop(L)))
}
TEST_NOTEQUAL_INT(lua_pcall(L, 0, LUA_MULTRET, 0), 0, "Missing T_END");

lua_close(L);

TEST_END;

#endif/*TEST_LUAUTIL*/
