/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            translater_xml.cc
 *
 *  Tue Mar  3 10:37:19 CET 2009
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
#include "translater_xml.h"

#include <stdio.h> 
#include <hugin.hpp>

#include <xml_encode_decode.h>

static std::string output_value(TreeItem &item, std::string indent)
{
  return indent + "<value name=\"" + xml_encode(item.name) + "\">"
    + xml_encode(item.value) + "</value>\n";
}

static std::string output_group(TreeItem &group, std::string indent)
{
  std::string out;

  out += indent + "<group name=\"" + xml_encode(group.name) + "\">\n";

  TreeItems::iterator i = group.children.begin();
  while(i != group.children.end()) {
    if(i->isgroup) out += output_group(*i, indent + "  ");
    else out += output_value(*i, indent + "  ");
    i++;
  }

  out += indent + "</group>\n";

  return out;
}

std::string translate_xml(Trees &trees)
{
  std::string out;

  DEBUG(translater, "Prettyprinting output for %d trees\n",
        (int)trees.size());

  out += "<?xml version='1.0' encoding='utf-8'?>\n";
  out += "<results>\n";

  Trees::iterator i = trees.begin();
  while(i != trees.end()) {
    char ts[64];
    snprintf(ts, sizeof(ts), "%lu", i->timestamp);

    char sid[64];
    snprintf(sid, sizeof(sid), "%u", i->id);

    char errcode[64];
    snprintf(errcode, sizeof(errcode), "%u", i->errcode);

    out +=
      "  <result id=\"" + std::string(sid) + "\" "
      "errcode=\"" + errcode + "\" "
      "errstring=\"" + i->errstring+"\" "
      "class=\"" + i->classname+"\"\n"
      "          deviceid=\"" + i->device_id+"\" "
      "devicetype=\"" + i->device_type+"\" "
      "timestamp=\"" + ts +  "\">\n";

    if(i->hasroot) {
      out += output_group(i->root, "    ");
    }

    out += "  </result>\n";
    i++;
  }
  out += "</results>\n";

  DEBUG(translater, "%s", out.c_str());

  return out;
}

#ifdef TEST_TRANSLATER_XML
//deps: ../lib/status.cc ../lib/debug.cc ../lib/log.cc ../lib/xml_encode_decode.cc
//cflags: -I../lib -I..
//libs:
#include "test.h"

TEST_BEGIN;



TEST_END;

#endif/*TEST_TRANSLATER_XML*/
