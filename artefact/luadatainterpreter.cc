/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luadatainterpreter.cc
 *
 *  Thu Feb 26 09:55:08 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#include "luadatainterpreter.h"

#include <hugin.hpp>

#include <luautil.h>
#include <config.h>

#include "luapostgresql.h"
#include "luatcpsocket.h"
#include "luaexpat.h"

#include <utf8.h>

#define UNIQUEID "interpreter"

// Declared in artefact.cc
extern std::string lua_basedir;

static int _getRoot(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_END);

  lua_pushnumber(L, 0);

  return 1;
}

static int _addGroup(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
 
  int gd = (int) lua_tonumber(L, lua_gettop(L) - 1);
  std::string name = lua_tostring(L, lua_gettop(L));

  LUADataInterpreter *c =
    (LUADataInterpreter*)Pentominos::getGlobal(L, UNIQUEID);

  gd = c->addGroup(gd, name);

  lua_pushnumber(L, gd);

  return 1;
}

static int _addValue(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_NUMBER,
                              Pentominos::T_STRING,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
  
  int gd = (int) lua_tonumber(L, lua_gettop(L) - 2);
  std::string name = lua_tostring(L, lua_gettop(L) - 1);
  std::string value = lua_tostring(L, lua_gettop(L));

  LUADataInterpreter *c =
    (LUADataInterpreter*)Pentominos::getGlobal(L, UNIQUEID);

  c->addValue(gd, name, value);

  return 0;
}

static int _setResume(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
 
  std::string resume = lua_tostring(L, lua_gettop(L));

  LUADataInterpreter *c =
    (LUADataInterpreter*)Pentominos::getGlobal(L, UNIQUEID);

  c->setResume(resume);

  return 0;
}

static int UTF8Encode(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
  
  Pentominos::UTF8 utf8;
  std::string str = utf8.encode(lua_tostring(L, lua_gettop(L)));
  lua_pushstring(L, str.c_str());
  return 1;
}

static int UTF8Decode(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
  
  Pentominos::UTF8 utf8;
  std::string str = utf8.decode(lua_tostring(L, lua_gettop(L)));
  lua_pushstring(L, str.c_str());
  return 1;
}

static int _getCPR(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_END);
  
  LUADataInterpreter *c =
    (LUADataInterpreter*)Pentominos::getGlobal(L, UNIQUEID);
  lua_pushstring(L, c->getCPR().c_str());

  return 1;
}

static int _luaBaseDir(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_END);

  lua_pushstring(L, lua_basedir.c_str());
  return 1;
}

static int _printInterpreterMessage(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);

  std::string msg = lua_tostring(L, lua_gettop(L));
  DEBUG(luadatainterpreter, "%s", msg.c_str());

  return 1;
}

LUADataInterpreter::LUADataInterpreter(Pentominos::Entry &e, bool r)
  : LUA(UNIQUEID), entry(e), resume(r)
{
  tree.timestamp = time(NULL);
  tree.hasroot = false;

  lua_register(L, "getRoot", _getRoot);
  lua_register(L, "addGroup", _addGroup);
  lua_register(L, "addValue", _addValue);
  lua_register(L, "setResume", _setResume);
  lua_register(L, "UTF8Encode", UTF8Encode);
  lua_register(L, "UTF8Decode", UTF8Decode);
  lua_register(L, "getCPR", _getCPR);
  lua_register(L, "printInterpreterMessage", _printInterpreterMessage);
  lua_register(L, "luaBaseDir", _luaBaseDir);

  register_postgresql(L);
  register_tcpsocket(L);
  register_expat(L);
}

LUADataInterpreter::~LUADataInterpreter()
{
}

void LUADataInterpreter::interpret(Pentominos::Data &data,
                                   std::string classname)
{
  tree.classname = classname;
  tree.timestamp = data.timestamp;
  tree.device_id = data.device_id;
  tree.device_type = data.device_type;
  tree.errcode = 0;
  tree.errstring = "";

  // Load LUA file
  std::string program  = lua_basedir + "/" + data.device_type + ".lua";

  DEBUG(luadatainterpreter, "Loading Interpreter: %s\n",
                   program.c_str());

  if(luaL_loadfile(L, program.c_str())) {
    std::string message = lua_tostring(L, lua_gettop(L));
    error(message);
    throw LUADataInterpreterException(message);
  }

  // Run program (init)
  if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
    std::string message = lua_tostring(L, lua_gettop(L));
    error(message);
    throw LUADataInterpreterException(message);
  }

  std::string datapath;
  try {
    std::string d = Pentominos::config["datapath"];
    datapath = d;
  } catch( ... ) {
    std::string message = "Could not read 'datapath' from config file.";
    error(message);
    throw LUADataInterpreterException(message);
  }

  if(datapath.at(datapath.length() - 1) != '/') datapath.append("/");
  
  std::string fullfilename = datapath + "/" + data.filename;

  lua_getglobal(L, "interpret");
  lua_pushstring(L, fullfilename.c_str());
  lua_pushstring(L, classname.c_str());
  if(lua_pcall(L, 2, LUA_MULTRET, 0)) {
    std::string message = lua_tostring(L, lua_gettop(L));
    error(message);
    throw LUADataInterpreterException(message);
  }
}

void LUADataInterpreter::static_class(std::string classname)
{
  tree.classname = classname;
  tree.timestamp = time(NULL);
  tree.device_id = "";
  tree.device_type = "";
  tree.errcode = 0;
  tree.errstring = "";

  // Load LUA file
  std::string program  = lua_basedir + "/" + classname + ".lua";

  DEBUG(luadatainterpreter, "Loading static Interpreter: %s\n",
                   program.c_str());

  if(luaL_loadfile(L, program.c_str())) {
    error(lua_tostring(L, lua_gettop(L)));
  }

  // Run program (init)
  lua_pcall(L, 0, LUA_MULTRET, 0);
}

gd_t LUADataInterpreter::addGroup(gd_t parent, std::string name)
{
  if(parent == 0 && tree.hasroot == false) {
    // Push and return the tree root group.
    if(groups.size() != 0) {
      ERR(luadatainterpreter,
                     "ERROR: Tree already has a root group\n");
      return -1;
    }

    groups.push_back(&tree.root);
    tree.root.name = name;
    tree.root.isgroup = true;
    tree.hasroot = true;

    return 0;
  }

  if(parent < 0 || parent > ((int) groups.size()) - 1) {
    ERR(luadatainterpreter,
                   "ERROR: No such parent group %d, trying to insert '%s'\n",
                   parent, name.c_str());
    return -1;
  }

  TreeItem *ti = groups[parent];

  TreeItem child;
  child.name = name;
  child.isgroup = true;
  ti->children.push_back(child);

  gd_t idx = groups.size();
  groups.push_back(&(ti->children.back()));
  
  return idx;
}

void LUADataInterpreter::addValue(gd_t parent, std::string name,
                                  std::string value)
{
  // In resume mode, only show the resume.
  //if(resume && name != "resume") return;
 
//  printf("Adding value (%s, %s)\n", name.c_str(), value.c_str());

  if(parent < 0 || parent > (int) groups.size() - 1) {
    ERR(luadatainterpreter,
                   "ERROR: No such parent group %d,"
                   " trying to insert '%s' => '%s'\n",
                   parent, name.c_str(), value.c_str());
    return;
  }

  TreeItem child;
  child.name = name;
  child.value = value;
  child.isgroup = false;
  groups[parent]->children.push_back(child);
}

void LUADataInterpreter::setResume(std::string resumetxt)
{
  if(!resume) return;

  addValue(0/*root*/, "resume", resumetxt);
}

std::string LUADataInterpreter::getCPR()
{
  return entry.patientid;
}

void LUADataInterpreter::error(std::string message)
{
  ERR(luadatainterpreter, "LUADataInterpreter::error (%s)\n",
      message.c_str());
}

Tree LUADataInterpreter::getTree()
{
  return tree;
}
