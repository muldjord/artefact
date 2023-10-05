/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            resultparser.cc
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
#include "resultparser.h"

#include <string.h>

ResultParser::ResultParser()
{
  _root = NULL;
}

ResultParser::~ResultParser()
{
}

atf_result_node_t *ResultParser::root()
{
  return _root;
}

void ResultParser::appendSibling(atf_result_node_t *node)
{
  atf_result_node_t *parent = NULL;
  if(stack.size()) parent = stack[stack.size() - 1];

  if(parent) {
    if(parent->child) {
      atf_result_node_t *sibling = parent->child;
      atf_result_node_t *last = sibling;
      while(sibling) {
        last = sibling;
        sibling = sibling->next;
      }
      last->next = node;
    } else {
      parent->child = node;
    }
  }
}

void ResultParser::startTag(std::string name,
                            std::map< std::string, std::string> attr)
{
  if(name == "group") {

    if(attr.find("name") == attr.end()) {
      throw BadContentException();
    }

    atf_result_node_t *node = new atf_result_node_t;

    node->name = strdup(attr["name"].c_str());
    node->value = NULL;
    node->child = NULL;
    node->next = NULL;

    appendSibling(node);

    stack.push_back(node);
    if(_root == NULL) _root = node;

    return;
  }

  if(name == "value") {

    if(attr.find("name") == attr.end()) {
      throw BadContentException();
    }

    if(_root == NULL) throw BadContentException();
    
    atf_result_node_t *node = new atf_result_node_t;

    node->name = strdup(attr["name"].c_str());
    // node->value = ... Is set later
    node->child = NULL;
    node->next = NULL;

    appendSibling(node);

    this->valnode = node;
    this->value = "";

    return;
  }

  throw BadContentException();
}

void ResultParser::characterData(std::string &cdata)
{
  if(this->valnode) value += cdata;
}

void ResultParser::endTag(std::string name)
{
  if(name == "group") {
    stack.pop_back();
    return;
  }

  if(name == "value") {
    valnode->value = strdup(value.c_str());
    this->value = "";
    this->valnode = NULL;
    return;
  }

  throw BadContentException();
}

void ResultParser::parseError(const char *buf, size_t len,
                              std::string error, int lineno)
{
  throw ParsingException();
  /*
  char *zbuf = (char*)calloc(sizeof(char), len + 1);
  memcpy(buf, zbuf, len * sizeof(char));

  ERR(xmlparser, "ResultParser error at line %d: %s\n", lineno, error.c_str());
  ERR(xmlparser, "\tBuffer %u bytes: [%s]\n", len, zbuf);

  free(zbuf);
  */
}

#ifdef TEST_RESULTPARSER
//deps: ../lib/saxparser.cc ../lib/debug.cc ../lib/log.cc
//cflags: -I../lib -I.. $(EXPAT_CFLAGS)
//libs: $(EXPAT_LIBS)
#include "test.h"

#define GOOD_XML                                    \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"    \
  "\n"                                              \
  "    <group name=\"group1\">\n"                   \
  "      <value name=\"name1\">value1</value>\n"    \
  "      <value name=\"name2\">value2</value>\n"    \
  "      <group name=\"group2\">\n"                 \
  "        <value name=\"name3\">value3</value>\n"  \
  "        <value name=\"name4\">value4</value>\n"  \
  "      </group>\n"                                \
  "      <group name=\"group3\">\n"                 \
  "      </group>\n"                                \
  "    </group>\n"                                  \
  "  "

#define GOOD_XML_BAD_CONTENT                        \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"    \
  "\n"                                              \
  "    <dims name=\"echo\">\n"                      \
  "      <dut name=\"hello\" value=\"world\"/>\n"   \
  "    </dims>\n"                                   \
  "  "

#define BAD_XML                                     \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"    \
  "\n"                                              \
  "    <group name=\"echo\">\n"                     \
  "      <value name=\"hello\" value=\"world\"/>\n" \
  "    </gourp>\n"                                  \
  "  "

TEST_BEGIN;
{ // Positive tests
  ResultParser parser;

  TEST_NOEXCEPTION(parser.parse(GOOD_XML, strlen(GOOD_XML)), "No exception...");
  TEST_NOTEQUAL(parser.root(), NULL,
                "Test if we got some DOM nodes.");
  
  atf_result_node_t *group1 = parser.root();
  TEST_EQUAL_STR(std::string(group1->name), "group1", "Test name");
  TEST_EQUAL(group1->value, NULL, "Test if we are a group.");
  TEST_NOTEQUAL(group1->child, NULL, "Test if we have a child.");
  TEST_EQUAL(group1->next, NULL, "Test if we have siblings.");

  atf_result_node_t *name1 = group1->child;
  TEST_EQUAL_STR(std::string(name1->name), "name1", "Test name");
  TEST_EQUAL_STR(std::string(name1->value), "value1", "Test value");
  TEST_EQUAL(name1->child, NULL, "Test if we have a child.");
  TEST_NOTEQUAL(name1->next, NULL, "Test if we have siblings.");

  atf_result_node_t *name2 = name1->next;
  TEST_EQUAL_STR(std::string(name2->name), "name2", "Test name");
  TEST_EQUAL_STR(std::string(name2->value), "value2", "Test value");
  TEST_EQUAL(name2->child, NULL, "Test if we have a child.");
  TEST_NOTEQUAL(name2->next, NULL, "Test if we have siblings.");

  atf_result_node_t *group2 = name2->next;
  TEST_EQUAL_STR(std::string(group2->name), "group2", "Test name");
  TEST_EQUAL(group2->value, NULL, "Test if we are a group.");
  TEST_NOTEQUAL(group2->child, NULL, "Test if we have a child.");
  TEST_NOTEQUAL(group2->next, NULL, "Test if we have siblings.");

  atf_result_node_t *name3 = group2->child;
  TEST_EQUAL_STR(std::string(name3->name), "name3", "Test name");
  TEST_EQUAL_STR(std::string(name3->value), "value3", "Test value");
  TEST_EQUAL(name3->child, NULL, "Test if we have a child.");
  TEST_NOTEQUAL(name3->next, NULL, "Test if we have siblings.");

  atf_result_node_t *name4 = name3->next;
  TEST_EQUAL_STR(std::string(name4->name), "name4", "Test name");
  TEST_EQUAL_STR(std::string(name4->value), "value4", "Test value");
  TEST_EQUAL(name4->child, NULL, "Test if we have a child.");
  TEST_EQUAL(name4->next, NULL, "Test if we have siblings.");

  atf_result_node_t *group3 = group2->next;
  TEST_EQUAL_STR(std::string(group3->name), "group3", "Test name");
  TEST_EQUAL(group3->value, NULL, "Test if we are a group.");
  TEST_EQUAL(group3->child, NULL, "Test if we have a child.");
  TEST_EQUAL(group3->next, NULL, "Test if we have siblings.");
}

{ // Negative test
  ResultParser parser;
  TEST_EXCEPTION(parser.parse(GOOD_XML_BAD_CONTENT,
                              strlen(GOOD_XML_BAD_CONTENT)),
                 ResultParser::BadContentException, "Test");
}

{ // Negative test
  ResultParser parser;
  TEST_EXCEPTION(parser.parse(BAD_XML, strlen(BAD_XML)),
                 ResultParser::ParsingException, "Test");
}

TEST_END;

#endif/*TEST_RESULTPARSER*/
