/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            xmlparser.cc
 *
 *  Wed Mar 28 09:47:17 CEST 2007
 *  Copyright  2006 Bent Bisballe Nyeng
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
#include "xmlparser.h"

#include <hugin.hpp>

#include <stdio.h>
#include <string.h>
#include <expat.h>

#include <string>
#include <map>

#include "utf8.h"

XMLParser::XMLParser()
{
  data = NULL;

  // I leave this here (the 'p = 0'), just to remind me never to make 
  // such a major fuckup again! (It overwrites a variable in the 
  // superclass, which was not declared private as it should be)

  //  p = 0;

  //  logger = new TransactionLogger(entry);

  b64dec = NULL;
}

XMLParser::~XMLParser()
{
  if(data) delete data;
  if(b64dec) delete b64dec;
  //  delete logger;
}

void XMLParser::characterData(std::string &cdata)
{
  if(!b64dec) return;
  
  Pentominos::Data *dataentry = NULL;
  if(entry.datalist.size()) {
    dataentry = entry.datalist[entry.datalist.size() - 1];
  }

  if(data && dataentry && dataentry->encoding == "base64") {
    std::string d = b64dec->write(cdata);
    data->append(d.data(), d.length());
  }
}

void XMLParser::startTag(std::string name,
                         std::map< std::string, std::string> attr)
{
  if(name == "pentominos:entry") {

    if(attr.find("patientid") == attr.end() ||
       attr.find("timestamp") == attr.end()) {
      // Missing required attribute
      ERR(xmlparser, "Missing 'entry' attribute\n");
      return;
    }

    entry.setPatientID(attr["patientid"]);
    entry.timestamp = atoll(attr["timestamp"].c_str());
    entry.pretend = attr.find("pretend") != attr.end() &&
      attr["pretend"] == "true";
    DEBUG(xmlparser, "Pretend == %s\n", entry.pretend?"true":"false");
  }

  if(name == "pentominos:data") {

    if(attr.find("id") == attr.end() ||
       attr.find("operator") == attr.end() ||
       attr.find("device_type") == attr.end() ||
       attr.find("device_id") == attr.end() ||
       attr.find("timestamp") == attr.end() ||
       attr.find("location") == attr.end() ||
       attr.find("encoding") == attr.end()) {
      // Missing required attribute
      ERR(xmlparser, "Missing 'data' attribute\n");
      return;
    }

    // Create a new dataobject.
    Pentominos::Data *d = new Pentominos::Data();
    d->id = atoi(attr["id"].c_str());
    d->operatorid = attr["operator"];
    d->device_type = attr["device_type"];
    d->device_id = attr["device_id"];
    d->timestamp = atoll(attr["timestamp"].c_str());
    d->location = attr["location"];
    d->encoding = attr["encoding"];

    entry.datalist.push_back(d);

    if(data) delete data;
    data = new DataWriter(d);
    
    if(b64dec) delete b64dec;
    b64dec = new Pentominos::Base64Decoder();
  }

  if(name == "pentominos:query") {

    if(attr.find("id") == attr.end() ||
       attr.find("class") == attr.end()) {
      // Missing required attribute
      ERR(xmlparser, "Missing 'query' attribute\n");
      return;
    }

    // Create a new query
    Pentominos::Query *query = new Pentominos::Query();

    query->id = atoi(attr["id"].c_str());
    query->cls = attr["class"];

    if(attr.find("from_time") != attr.end()) {
      query->from_time = atoll(attr["from_time"].c_str());
      query->from_time_use = true;
    } else {
      query->from_time = 0;
      query->from_time_use = false;
    }

    if(attr.find("to_time") != attr.end()) {
      query->to_time = atoll(attr["to_time"].c_str());
      query->to_time_use = true;
    } else {
      query->to_time = 0;
      query->to_time_use = false;
    }

    if(attr.find("filter") != attr.end()) {
      if(attr["filter"] == "all") query->filter = Pentominos::QF_ALL;
      else if(attr["filter"] == "latest")
        query->filter = Pentominos::QF_LATEST;
      else {
        ERR(xmlparser,
                       "Invalid value of 'filter' : %s using QF_ALL\n",
                       attr["filter"].c_str());
        query->filter = Pentominos::QF_ALL;
      }
    } else {
      query->filter = Pentominos::QF_ALL;
    }

    entry.querylist.push_back(query);
  }
}

void XMLParser::endTag(std::string name)
{
  if(name == "pentominos:data") {
    DEBUG(xmlparser, "EOF cdata\n");
    if(data) {
      delete data;
      data = NULL;
      if(b64dec) delete b64dec;
      b64dec = new Pentominos::Base64Decoder();
    }
  }

  if(name == "pentominos:query") {
  }

  if(name == "artefact") {
  }
}

void XMLParser::parseError(const char *buf, size_t len,
                           std::string error, int lineno)
{
  char *zbuf = (char*)calloc(sizeof(char), len + 1);
  memcpy(zbuf, buf, len * sizeof(char));

  ERR(xmlparser, "XMLParser error at line %d: %s\n",
                 lineno, error.c_str());
  ERR(xmlparser, "\tBuffer %u bytes: [%s]\n", (int)len, zbuf);
  
  free(zbuf);
}
