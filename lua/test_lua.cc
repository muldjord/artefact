/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            test_lua.cc
 *
 *  Wed Jan 20 11:52:28 CET 2010
 *  Copyright 2010 Jonas Suhr Christensen
 *  jsc@umbraculum.org
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
#include <vector>
#include <string>

#include <lua.h>

#include <cppunit/extensions/HelperMacros.h>

#include "../lib/lua.h"
#include "../lib/luautil.h"
//#include "../tools/test.h"

static int TEST_num_fails = 0;
//static int TEST_num_tests = 0;

static int _TEST_EQUAL_STR(lua_State *L) {
  int n = lua_gettop(L);
  if(n != 3) {
    char errstr[512];
    sprintf(errstr, "Expecting 3 arguments, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }

  std::string comment = lua_tostring(L, lua_gettop(L));

  // Checking argument types
  if(!lua_isstring(L, lua_gettop(L) - 2) || 
     !lua_isstring(L, lua_gettop(L) - 1))
  {
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-2));
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-1));
//    TEST_TRUE(lua_isstring(L, lua_gettop(L) -2), "%s", comment.c_str());
//    TEST_TRUE(lua_isstring(L, lua_gettop(L) -1), "%s", comment.c_str());
    return 0; // Wrong argument type
  }

  std::string first_argument = lua_tostring(L, lua_gettop(L) - 2);
  std::string second_argument = lua_tostring(L, lua_gettop(L) - 1);

  CPPUNIT_ASSERT_EQUAL_MESSAGE(comment, first_argument, second_argument);
//  TEST_EQUAL_STR(first_argument, second_argument, "%s", comment.c_str());

  return 1;
}

static int _TEST_NOTEQUAL_STR(lua_State *L) {
  int n = lua_gettop(L);
  if(n != 3) {
    char errstr[512];
    sprintf(errstr, "Expecting 3 arguments, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }

  std::string comment = lua_tostring(L, lua_gettop(L));

  // Checking argument types
  if(!lua_isstring(L, lua_gettop(L) - 2) || 
     !lua_isstring(L, lua_gettop(L) - 1))
  {
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-2));
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-1));
//    TEST_TRUE(lua_isstring(L, lua_gettop(L) -2), "%s", comment.c_str());
//    TEST_TRUE(lua_isstring(L, lua_gettop(L) -1), "%s", comment.c_str());
    return 0; // Wrong argument type
  }

  std::string first_argument = lua_tostring(L, lua_gettop(L) - 2);
  std::string second_argument = lua_tostring(L, lua_gettop(L) - 1);

  CPPUNIT_ASSERT_MESSAGE(comment, first_argument != second_argument);
//  TEST_NOTEQUAL_STR(first_argument, second_argument, "%s", comment.c_str()); 

  return 1;
}

static int _TEST_EQUAL_INT(lua_State *L) {
  int n = lua_gettop(L);
  if(n != 3) {
    char errstr[512];
    sprintf(errstr, "Expecting 3 arguments, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }
 
  std::string comment = lua_tostring(L, lua_gettop(L));

  // Checking argument types
  if(!lua_isnumber(L, lua_gettop(L) - 2) || 
     !lua_isnumber(L, lua_gettop(L) - 1))
  {
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-2));
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-1));
//    TEST_TRUE(lua_isnumber(L, lua_gettop(L) -2), "%s", comment.c_str());
//    TEST_TRUE(lua_isnumber(L, lua_gettop(L) -1), "%s", comment.c_str());
    return 0; // Wrong argument type
  }

  int first_argument = lua_tointeger(L, lua_gettop(L) - 2);
  int second_argument = lua_tointeger(L, lua_gettop(L) - 1);

  CPPUNIT_ASSERT_EQUAL_MESSAGE(comment, first_argument, second_argument);
//  TEST_EQUAL_INT(first_argument, second_argument, "%s", comment.c_str());

  return 1;
}

static int _TEST_NOTEQUAL_INT(lua_State *L) {
  int n = lua_gettop(L);
  if(n != 3) {
    char errstr[512];
    sprintf(errstr, "Expecting 3 arguments, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }

  std::string comment = lua_tostring(L, lua_gettop(L));
  
  // Checking argument types
  if(!lua_isnumber(L, lua_gettop(L) - 2) || 
     !lua_isnumber(L, lua_gettop(L) - 1))
  {

    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-2));
    CPPUNIT_ASSERT_MESSAGE(comment, lua_isstring(L, lua_gettop(L)-1));

//    TEST_TRUE(lua_isnumber(L, lua_gettop(L) -2), "%s", comment.c_str());
//    TEST_TRUE(lua_isnumber(L, lua_gettop(L) -1), "%s", comment.c_str());
    return 0; // Wrong argument type
  }

  int first_argument = lua_tointeger(L, lua_gettop(L) - 2);
  int second_argument = lua_tointeger(L, lua_gettop(L) - 1);
  CPPUNIT_ASSERT_MESSAGE(comment, first_argument != second_argument);
//  CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT_EQUAL_MESSAGE(comment, first_argument, second_argument));
//  TEST_NOTEQUAL_INT(first_argument, second_argument, "%s", comment.c_str());

  return 1;
}


static int _TEST_TRUE(lua_State *L) {

  int n = lua_gettop(L);

  if(n != 2) {
    char errstr[512];
    sprintf(errstr, "Expecting 2 arguments, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }

  std::string comment = lua_tostring(L, lua_gettop(L));
  
  bool first_argument = lua_toboolean(L, lua_gettop(L) - 1);

  CPPUNIT_ASSERT_MESSAGE(comment, first_argument == true);
//  TEST_TRUE(first_argument, "%s", comment.c_str());

  return 1;
}

static int _TEST_FALSE(lua_State *L) {

  int n = lua_gettop(L);

  if(n != 2) {
    char errstr[512];
    sprintf(errstr, "Expecting 2 argument, got %d", n);
    printf("%s\n", errstr);
    lua_pushstring(L, errstr);
    lua_error(L);
    return 0;
  }

  std::string comment = lua_tostring(L, lua_gettop(L) - 0);
  
  bool first_argument = lua_toboolean(L, lua_gettop(L) - 1);

  CPPUNIT_ASSERT_MESSAGE(comment, first_argument == false);
//  TEST_FALSE(first_argument, "%s", comment.c_str());

  return 1;
}

static std::string lua_file;
static lua_State *L;

int lua_setup() {
  L = luaL_newstate();
  if(L == NULL) return 0;
  
  luaL_openlibs(L);

  lua_register(L, "TEST_EQUAL_STR", _TEST_EQUAL_STR);
  lua_register(L, "TEST_NOTEQUAL_STR", _TEST_NOTEQUAL_STR);
  lua_register(L, "TEST_EQUAL_INT", _TEST_EQUAL_INT);
  lua_register(L, "TEST_NOTEQUAL_INT", _TEST_NOTEQUAL_INT);
  lua_register(L, "TEST_TRUE", _TEST_TRUE);
  lua_register(L, "TEST_FALSE", _TEST_FALSE);
  
  return 1;
}

int lua_load() {
  if(luaL_loadfile(L, lua_file.c_str())) {
    std::string errormsg = lua_tostring(L, lua_gettop(L));
    printf("Error loading file: %s\n\n%s\n", lua_file.c_str(), errormsg.c_str());
    return 0;
  }

  // Initialize lua functions (and run main code)
  if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
    std::string errormsg = lua_tostring(L, lua_gettop(L));
    printf("Error initializing lua file:\n\n%s\n", errormsg.c_str());
    return 0;
  }
  
  lua_getglobal(L, "test");
  if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
    std::string errormsg = lua_tostring(L, lua_gettop(L));
    printf("Error calling lua test function:\n\n%s\n", errormsg.c_str());
    return 0;
  }
  return 1;
}

int main(int argc, char* argv[]) {

  if(argc != 2) {
    return 1;
  }

  lua_file = std::string(argv[1]);
  
  printf("Loading test function from: %s\n", lua_file.c_str());

  lua_setup();
  lua_load();
  
  lua_close(L);
  
  return TEST_num_fails;
}
