/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luadatainterpreter.h
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
#ifndef __PENTOMINOS_LUADATAINTERPRETER_H__
#define __PENTOMINOS_LUADATAINTERPRETER_H__

#include <vector>
#include <string>

#include <entry.h>
#include <lua.h>

#include "tree.h"

#include <exception.h>

class LUADataInterpreterException : public Pentominos::Exception {
public:
  LUADataInterpreterException(std::string what) :
    Pentominos::Exception(what) {}
};

typedef int gd_t;

class LUADataInterpreter : public Pentominos::LUA
{
public:
  LUADataInterpreter(Pentominos::Entry &entry, bool resume);
  ~LUADataInterpreter();

  void interpret(Pentominos::Data &data, std::string classname);
  void static_class(std::string classname);

  gd_t addGroup(gd_t parent, std::string name);
  void addValue(gd_t group, std::string name, std::string value);
  void setResume(std::string resume);

  std::string getCPR();

  void error(std::string message);

  Tree getTree();

private:
  Tree tree;
  std::vector<TreeItem*> groups;
  Pentominos::Entry &entry;
  bool resume;
};

#endif/*__PENTOMINOS_LUADATAINTERPRETER_H__*/
