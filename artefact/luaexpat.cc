/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luaexpat.cc
 *
 *  Thu Mar  4 14:46:30 CET 2010
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
#include "luaexpat.h"

#include <expat.h>

#include <luautil.h>

typedef struct lxp_userdata {
  lua_State *L;
  XML_Parser parser;  // associated expat parser
  int tableref;  // table with callbacks for this parser
} lxp_userdata;

static void f_CharData(void *ud, const char *s, int len)
{
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;

  // get handler
  lua_pushstring(L, "CharacterData");
  lua_gettable(L, 3);
  if (lua_isnil(L, -1)) {  // no handler?
    lua_pop(L, 1);
    return;
  }

  lua_pushvalue(L, 1);  // push the parser (`self')
  lua_pushlstring(L, s, len);  // push Char data
  lua_call(L, 2, 0);  // call the handler
}

static void f_EndElement(void *ud, const char *name)
{
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;

  lua_pushstring(L, "EndElement");
  lua_gettable(L, 3);
  if (lua_isnil(L, -1)) {  // no handler?
    lua_pop(L, 1);
    return;
  }

  lua_pushvalue(L, 1);  // push the parser (`self')
  lua_pushstring(L, name);  // push tag name
  lua_call(L, 2, 0);  // call the handler
}

static void f_StartElement(void *ud,
                           const char *name,
                           const char **atts)
{
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;

  lua_pushstring(L, "StartElement");
  lua_gettable(L, 3);
  if (lua_isnil(L, -1)) {  // no handler?
    lua_pop(L, 1);
    return;
  }

  lua_pushvalue(L, 1);  // push the parser (`self')
  lua_pushstring(L, name);  // push tag name

  // create and fill the attribute table
  lua_newtable(L);
  while (*atts) {
    lua_pushstring(L, *atts++);
    lua_pushstring(L, *atts++);
    lua_settable(L, -3);
  }

  lua_call(L, 3, 0);  // call the handler
}

static int lxp_make_parser(lua_State *L)
{
  XML_Parser p;
  lxp_userdata *xpu;

  // (1) create a parser object
  xpu = (lxp_userdata *)lua_newuserdata(L,
      sizeof(lxp_userdata));

  // pre-initialize it, in case of errors
  xpu->tableref = LUA_REFNIL;
  xpu->parser = NULL;

  // set its metatable
  luaL_getmetatable(L, "Expat");
  lua_setmetatable(L, -2);

  // (2) create the Expat parser
  p = xpu->parser = XML_ParserCreate(NULL);
  if (!p)
    luaL_error(L, "XML_ParserCreate failed");

  // (3) create and store reference to callback table
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_pushvalue(L, 1);  // put table on the stack top
  xpu->tableref = luaL_ref(L, LUA_REGISTRYINDEX);

  // (4) configure Expat parser
  XML_SetUserData(p, xpu);
  XML_SetElementHandler(p, f_StartElement, f_EndElement);
  XML_SetCharacterDataHandler(p, f_CharData);
  return 1;
}

static int lxp_parse(lua_State *L)
{
  int status;
  size_t len;
  const char *s;
  lxp_userdata *xpu;

  // get and check first argument (should be a parser)
  xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");
  luaL_argcheck(L, xpu, 1, "expat parser expected");

  // get second argument (a string)
  s = luaL_optlstring(L, 2, NULL, &len);

  // prepare environment for handlers:
  // put callback table at stack index 3
  lua_settop(L, 2);
  lua_getref(L, xpu->tableref);
  xpu->L = L;  // set Lua state

  // call Expat to parse string
  status = XML_Parse(xpu->parser, s, (int)len, s == NULL);

  // return error code
  lua_pushboolean(L, status);
  return 1;
}

static int lxp_close(lua_State *L)
{
  lxp_userdata *xpu;

  xpu = (lxp_userdata *)luaL_checkudata(L, 1, "Expat");
  luaL_argcheck(L, xpu, 1, "expat parser expected");

  // free (unref) callback table
  luaL_unref(L, LUA_REGISTRYINDEX, xpu->tableref);
  xpu->tableref = LUA_REFNIL;

  // free Expat parser (if there is one)
  if (xpu->parser)
    XML_ParserFree(xpu->parser);
  xpu->parser = NULL;
  return 0;
}

static const struct luaL_reg lxp_meths[] =
{
  {"parse", lxp_parse},
  {"close", lxp_close},
  {"__gc", lxp_close},
  {NULL, NULL}
};

static const struct luaL_reg lxp_funcs[] =
{
  {"new", lxp_make_parser},
  {NULL, NULL}
};

void register_expat(lua_State *L)
{
  // create metatable
  luaL_newmetatable(L, "Expat");

  // metatable.__index = metatable
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);

  // register methods
  luaL_openlib (L, NULL, lxp_meths, 0);

  // register functions (only lxp.new)
  luaL_openlib (L, "lxp", lxp_funcs, 0);
}

#ifdef TEST_LUAEXPAT
//deps: 
//cflags: $(LUA_CFLAGS) $(EXPAT_CFLAGS) -I../lib
//libs: $(LUA_LIBS) $(EXPAT_LIBS)
#include "test.h"

#define LUAPROG                                                       \
  "local count = 0\n"                                                 \
  "\n"                                                                \
  "callbacks = {\n"                                                   \
  "    StartElement = function (parser, tagname)\n"                   \
  "      io.write('+ ', string.rep('  ', count), tagname, '\\\n')\n"  \
  "      count = count + 1\n"                                         \
  "      end,\n"                                                      \
  "\n"                                                                \
  "    EndElement = function (parser, tagname)\n"                     \
  "      count = count - 1\n"                                         \
  "      io.write('- ', string.rep('  ', count), tagname, '\\\n')\n"  \
  "      end,\n"                                                      \
  "  }\n"                                                             \
  "\n"                                                                \
  "function interpret(line)\n"                                        \
  "  p = lxp.new(callbacks)     -- create new parser\n"               \
  "  assert(p:parse(line))               -- parse the line\n"         \
  "  assert(p:parse('\\\n'))            -- add a newline  end\n"      \
  "  assert(p:parse())        -- finish document\n"                   \
  "  p:close()\n"                                                     \
  "end\n"                                                             \
  "\n"                                                                \
  "interpret('<tag><test1>blah</test1><test2/></tag>')\n"

#include <string>

TEST_BEGIN;

lua_State *L = luaL_newstate();
luaL_openlibs(L);

register_expat(L);

if(luaL_loadstring(L, LUAPROG)) {
  std::string msg = lua_tostring(L, lua_gettop(L));
  printf("loadstring\n%s\n", msg.c_str());
}

if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
  std::string msg = lua_tostring(L, lua_gettop(L));
  printf("pcall\n%s\n", msg.c_str());
}

lua_close(L);

TEST_END;

#endif/*TEST_LUAEXPAT*/
