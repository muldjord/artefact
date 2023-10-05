/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luadataclassifier.cc
 *
 *  Thu Feb 26 09:55:23 CET 2009
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
#include "luadataclassifier.h"

#include <hugin.hpp>

#include <luautil.h>

#include <config.h>

#define UNIQUEID "classifier"

// Declared in artefact.cc
extern std::string lua_basedir;

static int _addClass(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);

  std::string classname = lua_tostring(L, lua_gettop(L));

  LUADataClassifier *c = (LUADataClassifier*)Pentominos::getGlobal(L, UNIQUEID);

  c->addClass(classname);

  return 0;
}

static int _invalidData(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);
  
  std::string errorstr = std::string("Data is invalid: ") +
    lua_tostring(L, lua_gettop(L));

  LUADataClassifier *c = (LUADataClassifier*)Pentominos::getGlobal(L, UNIQUEID);

  c->error(errorstr);

  lua_pushstring(L, errorstr.c_str());
  lua_error(L);

  return 0;
}

static int _printClassifierMessage(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_STRING,
                              Pentominos::T_END);

  std::string msg = lua_tostring(L, lua_gettop(L));
  DEBUG(luadataclassifier, "%s", msg.c_str());

  return 1;
}


static int _luaBaseDir(lua_State *L)
{
  Pentominos::checkParameters(L,
                              Pentominos::T_END);

  lua_pushstring(L, lua_basedir.c_str());
  return 1;
}

LUADataClassifier::LUADataClassifier()
  : LUA(UNIQUEID)
{
  lua_register(L, "addClass", _addClass);
  lua_register(L, "invalidData", _invalidData);
  lua_register(L, "printClassifierMessage", _printClassifierMessage);
  lua_register(L, "luaBaseDir", _luaBaseDir);
}

LUADataClassifier::~LUADataClassifier()
{
}

ClassList LUADataClassifier::classify(Pentominos::Data &data)
{
  classlist.clear();

  // Load LUA file
  std::string program  = lua_basedir + "/" + data.device_type + ".lua";

  INFO(luadataclassifier, "Loading Classifier: %s\n", program.c_str());

  if(luaL_loadfile(L, program.c_str())) {
    std::string message = lua_tostring(L, lua_gettop(L));
    error(message);
    throw LUADataClassifierException(message);
  }

  // Run program (init)
  if(lua_pcall(L, 0, LUA_MULTRET, 0)) {
    throw LUADataClassifierException(lua_tostring(L, lua_gettop(L)));
  }

  std::string datapath;
  try {
    std::string d = Pentominos::config["datapath"];
    datapath = d;
  } catch( ... ) {
    std::string message = "Could not read 'datapath' from config file.";
    error(message);
    throw LUADataClassifierException(message);
  }

  if(datapath.at(datapath.length() - 1) != '/') datapath.append("/");
  
  std::string fullfilename = datapath + "/" + data.filename;

  // Get function
  lua_getglobal(L, "classify");
  lua_pushstring(L, fullfilename.c_str());
  if(lua_pcall(L, 1, LUA_MULTRET, 0)) {
    throw LUADataClassifierException(lua_tostring(L, lua_gettop(L)));
  }

  return classlist;
}

void LUADataClassifier::addClass(Class c)
{
  classlist.push_back(c);
}

void LUADataClassifier::error(std::string message)
{
  ERR(luadataclassifier, "LUADataClassifier::error (%s)\n", message.c_str());
}
