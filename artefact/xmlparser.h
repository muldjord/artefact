/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            xmlparser.h
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
#ifndef __ARTEFACT_XMLPARSER_H__
#define __ARTEFACT_XMLPARSER_H__

#include <saxparser.h>

#include <base64.h>

#include "entry.h"
#include "exception.h"
//#include "transactionlogger.h"
#include "datawriter.h"

class XMLParserException : public Pentominos::Exception {
public:
	XMLParserException(std::string reason) :
    Pentominos::Exception("XMLParser error: " + reason) {}
};

class XMLParser : public Pentominos::SAXParser {
public:
  XMLParser();
  ~XMLParser();

  void characterData(std::string &data);
  void startTag(std::string name, std::map< std::string, std::string> attributes);
  void endTag(std::string name);

  void parseError(const char *buf, size_t len, std::string error, int lineno);

  Pentominos::Entry entry;

private:
  //  TransactionLogger *logger;

  DataWriter *data;

  Pentominos::Base64Decoder *b64dec;
};

#endif/*__ARTEFACT_XMLPARSER_H__*/
