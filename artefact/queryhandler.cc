/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandler.cc
 *
 *  Wed Apr  4 09:18:46 CEST 2007
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
#include "queryhandler.h"

#include <hugin.hpp>

#include "luadatainterpreter.h"

#include "config.h"

#include <errno.h>
#include <stdio.h>

#include <string.h>

// Declared in artefact.cc
extern std::vector< std::string > static_classes;

std::map<std::string, std::vector<std::string> > pseudo_classes;

static void loadPseudoClasses()
{
  pseudo_classes.clear();

  Pentominos::VariantList vl = Pentominos::config["pseudo_classes"];
  Pentominos::VariantList::iterator i = vl.begin();
  while(i != vl.end()) {
    std::string pseudoclass = *i;

    if(Pentominos::config.find(pseudoclass) == Pentominos::config.end()) {
      ERR(queryhandler, "Could not lookup pseudoclass '%s' in list.\n",
          pseudoclass.c_str());
      i++;
      continue;
    }

    Pentominos::Configs pcgrp = Pentominos::config[pseudoclass];

    std::string name = pcgrp["name"];

    //    DEBUG(queryhandler, "%s (%s)\n", name.c_str(),
    //               pseudoclass.c_str());

    Pentominos::VariantList cl = pcgrp["classlist"];
    Pentominos::VariantList::iterator cli = cl.begin();
    while(cli != cl.end()) {
      std::string classname = *cli;
        
      //      DEBUG(queryhandler, "-> %s\n", classname.c_str());
        
      pseudo_classes[name].push_back(classname);
        
      cli++;
    }
      
    i++;
  }

}

static int recurdepth = 0;
static std::vector<std::string> getClassList(std::string cls,
                                             std::string indent = "")
{
  std::vector<std::string> classes;

  recurdepth++;
  if(recurdepth > 10) goto out;

  if(pseudo_classes.find(cls) != pseudo_classes.end()) {

    DEBUG(queryhandler, "%s-> (%s)\n", indent.c_str(), cls.c_str());

    std::vector<std::string>::iterator i = pseudo_classes[cls].begin();
    while(i != pseudo_classes[cls].end()) {
      std::vector<std::string> _c = getClassList(*i, indent + "\t");
      classes.insert(classes.end(), _c.begin(), _c.end());
      i++;
    }

  } else {
    DEBUG(queryhandler, "%s-> %s\n", indent.c_str(),
                     cls.c_str());
    classes.push_back(cls);
  }

out:
  recurdepth--;
  return classes;
}

QueryHandler::QueryHandler(Pentominos::Entry &e, PostgreSQLTransaction &p,
                           bool r)
  : entry(e), pgsql(p), resume(r)
{
}

Trees QueryHandler::handle(Pentominos::Query &query)
{
  Trees trees;

  DEBUG(queryhandler, "Handling:\n");
  query.print();

  // Map classname through pseudoclasses
  loadPseudoClasses();
  std::vector<std::string> classes;
  classes = getClassList(query.cls);

  std::list< Pentominos::Data > datalist;

  std::vector<std::string>::iterator cls = classes.begin();
  while(cls != classes.end()) {

    DEBUG(queryhandler, "##### Class: %s\n", cls->c_str());

    bool isstatic = false;
    std::vector< std::string >::iterator i = static_classes.begin();
    while(i != static_classes.end()) {
      if(*cls == *i) {
        try {
          LUADataInterpreter interpreter(entry, resume);
          interpreter.static_class(*cls);
          Tree t = interpreter.getTree();
          t.id = query.id;
          trees.push_back(t);
        } catch(...) {
          ERR(queryhandler, "interpreter.static_class\n");
        }
        isstatic = true;
      }
      i++;
    }
    
    if(!isstatic) {
      //
      // Lookup entry in db
      //
      std::list< Pentominos::Data > dl =
        pgsql.get(entry.patientid, *cls,
                  query.from_time_use, query.from_time,
                  query.to_time_use, query.to_time);

      
      std::list< Pentominos::Data >::iterator i = dl.begin();
      while(i != dl.end()) {
        Pentominos::Data & data = *i;
        data._class = *cls;
        i++;
      }

      datalist.splice(datalist.end(), dl);
    }

    cls++;
  }

  datalist.sort();

  switch(query.filter) {
  case Pentominos::QF_ALL:
    {
      std::list< Pentominos::Data >::iterator i = datalist.begin();
      while(i != datalist.end()) {
        try {
          LUADataInterpreter interpreter(entry, resume);
          interpreter.interpret(*i, i->_class);
          Tree t = interpreter.getTree();
          t.id = query.id;
          trees.push_back(t);
        } catch(...) {
          ERR(queryhandler, "interpreter.interpret\n");
        }
        i++;
      }
    }
    break;
    
  case Pentominos::QF_LATEST:
    {
      std::list< Pentominos::Data > results;
      
      // Find latest Data
      if(datalist.rbegin() != datalist.rend()) {
        Pentominos::Data &cur = *datalist.rbegin();
        
        // Find other Data matching the same uid.
        std::list< Pentominos::Data >::iterator i = datalist.begin();
        while(i != datalist.end()) {
          Pentominos::Data &pen = *i;
          if(cur.uid == pen.uid) results.push_back(pen);
          i++;
        }
      }
      
      // Produce result tree.
      std::list< Pentominos::Data >::iterator i = results.begin();
      while(i != results.end()) {
        try {
          LUADataInterpreter interpreter(entry, resume);
          interpreter.interpret(*i, i->_class);
          Tree t = interpreter.getTree();
          t.id = query.id;
          trees.push_back(t);
        } catch(...) {
          ERR(queryhandler, "interpreter.interpret\n");
        }
        i++;
      }
    }
    break;
    
  default:
    // ERROR!
    break;
  }

  return trees;
}

