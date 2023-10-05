/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            requestparser.cc
 *
 *  Thu Mar  4 14:48:10 CET 2010
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
#include "requestparser.h"

// Request:
/*
<request>
	<cpr></cpr>
	<start></start>
	<end></end>
	<categories>
		<category></category>
			.
			.
			.
	</categories>
</request>
*/
// <start> and <end> has the format YYYYMMDD

static time_t toTimestamp(std::string &data, bool startofday)
{
  if(data.length() != 8) return ((time_t)-1);

  struct tm _tm;

  _tm.tm_sec = 0;

  if(startofday) {
    _tm.tm_min = 0;
    _tm.tm_hour = 0;
  } else {
    _tm.tm_min = 59;
    _tm.tm_hour = 23;
  }
    
  _tm.tm_mday = atol(data.substr(6, 2).c_str()); // range [1-31] 
  _tm.tm_mon = atol(data.substr(4, 2).c_str()) - 1; // range [0-11]
  _tm.tm_year = atol(data.substr(0, 4).c_str()) - 1900;
  _tm.tm_isdst = 0; // Not in daylight saving time.

  /*
  printf("DATE: %s (%d %d %d) => %lu\n",
         data.c_str(),
         _tm.tm_mday, _tm.tm_mon + 1, _tm.tm_year + 1900,
         mktime(&_tm));
  */
  return mktime(&_tm);
}

RequestParser::RequestParser()
{
  request.has_start = false;
  request.has_end = false;
}

void RequestParser::characterData(std::string &data)
{
  this->data += data;
}

void RequestParser::startTag(std::string name,
                             std::map< std::string, std::string> attributes)
{
  data = "";
}

void RequestParser::endTag(std::string name)
{
	if(name == "cpr") {
    request.patientid = data;
  }

	if(name == "start") {
    request.start = toTimestamp(data, true);
    request.has_start = true;
  }

	if(name == "end") {
    request.end = toTimestamp(data, false);
    request.has_end = true;
  }

	if(name == "categories") {}

	if(name == "category") {
    request.categories.push_back(data);
  }
}

void RequestParser::parseError(const char *buf, size_t len,
                               std::string error, int lineno)
{
}

#ifdef TEST_REQUESTPARSER
//deps:
//cflags:
//libs:
#include "test.h"

TEST_BEGIN;

TEST_END;

#endif/*TEST_REQUESTPARSER*/
