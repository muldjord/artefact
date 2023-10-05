/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            requestparser.h
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
#ifndef __PENTOMINOS_REQUESTPARSER_H__
#define __PENTOMINOS_REQUESTPARSER_H__

#include <saxparser.h>

#include "request.h"


class RequestParser : public Pentominos::SAXParser {
public:
  RequestParser();

  Request request;

  // Private callbacks:
  void characterData(std::string &data);

  void startTag(std::string name,
                std::map< std::string, std::string> attributes);

  void endTag(std::string name);

  void parseError(const char *buf, size_t len,
                  std::string error, int lineno);

private:
  std::string data;
};

#endif/*__PENTOMINOS_REQUESTPARSER_H__*/
