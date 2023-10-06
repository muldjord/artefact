/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tree.h
 *
 *  Tue Mar  3 08:37:02 CET 2009
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
#ifndef __PENTOMINOS_TREE_H__
#define __PENTOMINOS_TREE_H__

#include <vector>
#include <string>
#include <stdio.h>
#include <status.h>
#include <ctime>

typedef unsigned int id_t;

class TreeItem;
typedef std::vector< TreeItem > TreeItems;

class TreeItem {
public:
  std::string name;
  std::string value;
  TreeItems children;
  bool isgroup;
};

class Tree {
public:
  id_t id;

  time_t timestamp;

  unsigned int errcode;
  std::string errstring;

  std::string device_id;
  std::string device_type;

  std::string classname;

  bool hasroot;
  TreeItem root;
};

typedef std::vector< Tree > Trees;

inline Tree statusTree(id_t id, status_t errid, std::string msg)
{
  Tree t;

  char erridstr[256];
  sprintf(erridstr, "%d", errid);

  t.timestamp = time(NULL);
  t.classname = "status";
  t.id = id;
  t.errcode = errid;
  t.errstring = msg;
  t.hasroot = false;

  return t;
}

/*
Structure in pseudo BNF
-----------------------

 Results:
  Result *

 Result:
  ValueType *
  - ErrorCode / ErrorMessage
  - ResultID (maps to query ID,
               NOTE: several results can map to the same query id)
  - Timestamp (time of value production, eg. measurement)
  - Apparatus ID
  - Apparatus Type
  - Response Class (classname generating the result)

  ValueType:
   Group *
   Value *

  Group:
   ValueType *
   - name

  Value:
   - name
   - value (as character data)

XML example:
<results>
  <result err="0" errmsg="" id="42" timestamp="1234567890"
          devid="A243" devtype="lensmeter" class="visus">
    <group name="dims">
      <value name="ladida">some value</value>
      <value name="limbo">another value</value>
      <group name="edut">
        <value name="ladida">some value</value>
        <value name="limbo">another value</value>
      </group>
    </group>
  </result>
  <result err="0" errmsg="" id="42" timestamp="1234567890"
          devid="A243" devtype="lensmeter" class="visus">
    <group name="dims">
      <value name="ladida">some value</value>
      <value name="limbo">another value</value>
      <group name="edut">
        <value name="ladida">some value</value>
        <value name="limbo">another value</value>
      </group>
    </group>
  </result>
</results>

 */

#endif/*__PENTOMINOS_TREE_H__*/
