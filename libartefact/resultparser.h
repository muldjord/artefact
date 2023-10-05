/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            resultparser.h
 *
 *  Thu Feb 11 13:43:37 CET 2010
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
#ifndef __PENTOMINOS_RESULTPARSER_H__
#define __PENTOMINOS_RESULTPARSER_H__

#include <saxparser.h>

#include <exception>
#include <vector>

#include "libartefact_private.h"

class ResultParser : public Pentominos::SAXParser {
public:
  class BadContentException : std::exception {};
  class ParsingException : std::exception {};

  ResultParser();
  ~ResultParser();

  void startTag(std::string name,
                std::map< std::string, std::string> attributes);

  void characterData(std::string &cdata);
  void endTag(std::string name);

  void parseError(const char *buf, size_t len, std::string error, int lineno);

  atf_result_node_t *root();

private:
  void appendSibling(atf_result_node_t *node);

  std::vector<atf_result_node_t *> stack;
  atf_result_node_t *_root;

  atf_result_node_t *valnode;
  std::string value;
};

#endif/*__PENTOMINOS_RESULTPARSER_H__*/
