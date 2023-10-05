/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            responsparser.cc
 *
 *  Thu Jan 28 11:20:21 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#include "responsparser.h"

#include <stdio.h>
#include <string.h>

#include <string>
#include <map>

#include <base64.h>

ResponsParser::ResponsParser(atf_handle_t *h, std::map<atf_id,
                             std::vector<struct _atf_result_t> > &r)
 : replies(r)
{
  handle = h;
  current_id = 0;
  inresult = 0;
}

ResponsParser::~ResponsParser()
{
}

void ResponsParser::startTag(std::string name,
                             std::map< std::string, std::string> attr)
{
  if(name == "results") {
    // Nothing to do here...
    return;
  }

  if(name == "result") {
    if(++inresult > 1) return; // Accept but ignore nested result tags.

    if(attr.find("id") == attr.end() ||
       attr.find("errcode") == attr.end() ||
       attr.find("errstring") == attr.end() ||
       attr.find("timestamp") == attr.end() ||
       attr.find("deviceid") == attr.end() ||
       attr.find("devicetype") == attr.end() ||
       attr.find("class") == attr.end()) {
      throw BadContentException();
    }

    long pos = getBytePos();

    atf_id id = atoi(attr["id"].c_str());

    struct _atf_result_t result;

    result.handle = handle;

    result.errcode = (atf_status_t)atoi(attr["errcode"].c_str());
    result.errstring = attr["errstring"];
    result.timestamp = atol(attr["timestamp"].c_str());

    result.device_id = attr["deviceid"];
    result.device_type = attr["devicetype"];
    result.classname = attr["class"];

    result.start = pos;
    result.stop = 0;

    result.size = 0;
    result.data = NULL;

    replies[id].push_back(result);

    current_id = id;
    return;
  }

  if(!inresult) throw BadContentException();
}

void ResponsParser::characterData(std::string &cdata)
{
}

void ResponsParser::endTag(std::string name)
{
  if(name == "result") {
    long pos = getBytePos();

    replies[current_id][replies[current_id].size()-1].stop = pos;

    inresult--;
  }
}

void ResponsParser::parseError(const char *buf, size_t len,
                               std::string error, int lineno)
{
  throw ParsingException();
}

#ifdef TEST_RESPONSPARSER
//deps: ../lib/saxparser.cc ../lib/debug.cc ../lib/log.cc
//cflags: -I../lib -I.. $(EXPAT_CFLAGS)
//libs: $(EXPAT_LIBS)
#include "test.h"

#define GOOD_XML                                                        \
  "<?xml version='1.0' encoding='utf-8'?>\n"                            \
  "<results>\n"                                                         \
  "  <result id=\"0\" errcode=\"0\" errstring=\"Data Success\" "        \
  "timestamp=\"1265972068\" deviceid=\"echo\" devicetype=\"echo\" "     \
  "class=\"echo\">\n"                                                   \
  "  </result>\n"                                                       \
  "  <result id=\"1\" errcode=\"0\" errstring=\"Data Success\" "        \
  "timestamp=\"1265972068\" deviceid=\"echo\" devicetype=\"echo\" "     \
  "class=\"echo\">\n"                                                   \
  "    <result id=\"1\" errcode=\"0\" errstring=\"\" timestamp=\"0\"/>\n" \
  "    <bogus/>\n"                                                      \
  "  </result>\n"                                                       \
  "  <result id=\"2\" errcode=\"0\" errstring=\"OK\" "                  \
  "timestamp=\"1265972068\" deviceid=\"echo\" devicetype=\"echo\" "     \
  "class=\"echo\">\n"                                                   \
  "    <group name=\"echo\">\n"                                         \
  "      <value name=\"hello\">world</value>\n"                         \
  "    </group>\n"                                                      \
  "  </result>\n"                                                       \
  "  <result id=\"3\" errcode=\"0\" errstring=\"OK\" "                  \
  "timestamp=\"1265972068\" deviceid=\"echo\" devicetype=\"echo\" "     \
  "class=\"echo\">\n"                                                   \
  "    <group name=\"echo\">\n"                                         \
  "      <value name=\"hello\">world</value>\n"                         \
  "    </group>\n"                                                      \
  "  </result>\n"                                                       \
  "</results>"

#define BAD_XML                                 \
  "<?xml version='1.0' encoding='utf-8'?>\n"    \
  "<results>\n"                                 \
  "</stluser>\n"

#define GOOD_XML_BAD_CONTENT                    \
  "<?xml version='1.0' encoding='utf-8'?>\n"    \
  "<fisk>\n"                                    \
  "</fisk>\n"

TEST_BEGIN;

{ // Positive tests
  std::map<atf_id, std::vector<struct _atf_result_t> > replies;
  ResponsParser p(NULL, replies);
  TEST_NOEXCEPTION(p.parse((char*)GOOD_XML, strlen((char*)GOOD_XML)),
                   "Verify that no exception while parsing good XML.");
  TEST_EQUAL_INT(replies.size(), 4, "Did we get all replies?");
  TEST_EQUAL_INT(replies[0].size(), 1, "Did we get all results in 0?");
  TEST_EQUAL_INT(replies[1].size(), 1, "Did we get all results in 1?");
  TEST_EQUAL_INT(replies[2].size(), 1, "Did we get all results in 2?");
  TEST_EQUAL_INT(replies[3].size(), 1, "Did we get all results in 3?");
  
}

{ // Negative test
  std::map<atf_id, std::vector<struct _atf_result_t> > replies;
  ResponsParser p(NULL, replies);
  TEST_EXCEPTION(p.parse((char*)BAD_XML, strlen(BAD_XML)),
                 ResponsParser::ParsingException,
                 "Verify exception while parsing bad XML.");
}

{ // Negative test
  std::map<atf_id, std::vector<struct _atf_result_t> > replies;
  ResponsParser p(NULL, replies);
  TEST_EXCEPTION(p.parse((char*)GOOD_XML_BAD_CONTENT,
                         strlen(GOOD_XML_BAD_CONTENT)),
                 ResponsParser::BadContentException,
                 "Verify exception while parsing bad XML.");
}

TEST_END;

#endif/*TEST_RESPONSPARSER*/
