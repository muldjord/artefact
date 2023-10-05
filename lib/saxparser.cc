/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            saxparser.cc
 *
 *  Mon Mar 24 14:40:15 CET 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#include "saxparser.h"

#include <stdio.h>

#include <string.h>

static void character_hndl(void *p, const XML_Char *s, int len)
{
  Pentominos::SAXParser *parser = (Pentominos::SAXParser*)XML_GetUserData(p);
  std::string chars;
  chars.append(s, len);
  parser->characterData(chars);
}

static void start_hndl(void *p, const char *el, const char **attr)
{
  Pentominos::SAXParser *parser = (Pentominos::SAXParser*)XML_GetUserData(p);

  // Convert to comfy C++ values...
  std::string name = el;
  std::map< std::string, std::string > attributes;

  while(*attr) {
    std::string at_name = *attr;
    attr++;
    std::string at_value = *attr;
    attr++;

    attributes.insert(make_pair(at_name, at_value));
  }

  if(parser->outertag == "") parser->outertag = name;

  parser->startTag(name, attributes);
}

static void end_hndl(void *p, const char *el)
{
  Pentominos::SAXParser *parser = (Pentominos::SAXParser*)XML_GetUserData(p);
  std::string name = el;

  if(name == parser->outertag) parser->done = true;

  parser->endTag(name);
}


Pentominos::SAXParser::SAXParser()
{
  p = XML_ParserCreate(NULL);
  if(!p) {
    //ERR(sax, "Couldn't allocate memory for parser\n");
    //  throw std::exception(...);
    return;
  }

  XML_SetUserData(p, this);
  XML_UseParserAsHandlerArg(p);
  XML_SetElementHandler(p, start_hndl, end_hndl);
  XML_SetCharacterDataHandler(p, character_hndl);

  bufferbytes = 0;
  totalbytes = 0;
  done = false;

}

Pentominos::SAXParser::~SAXParser()
{
  if(p) XML_ParserFree(p);
}

int Pentominos::SAXParser::parse()
{
  char buf[32];
  int len;
  
  do {
    len = readData(buf, sizeof(buf) - 1);
    if (! XML_Parse(p, buf, len, len == 0)) {
      parseError(buf, len, XML_ErrorString(XML_GetErrorCode(p)), (int)XML_GetCurrentLineNumber(p));
      return 1;
    }

    memset(buf, 0, sizeof(buf));
  } while(len);

  return 0;
}

static bool iswhitespace(const char *buf, size_t size)
{
  for(size_t i = 0; i < size; i++)
    if(buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\t' && buf[i] != '\r')
      return false;
  return true;
}

bool Pentominos::SAXParser::parse(const char *data, size_t size)
{
  // DEBUG(sax, "parse %d bytes\n", size);

  bufferbytes = size;
  totalbytes += bufferbytes;

  if(! XML_Parse(p, data, size, false) ) {
    if(XML_GetErrorCode(p) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT) return true;
    if(XML_GetErrorCode(p) == XML_ERROR_FINISHED && iswhitespace(data, size)) return true;
    if(done && XML_GetErrorCode(p) == XML_ERROR_UNCLOSED_TOKEN) return true;
    parseError(data, size, XML_ErrorString(XML_GetErrorCode(p)), (int)XML_GetCurrentLineNumber(p));
    return false;
  }

  if(done) {
    if(! XML_Parse(p, data, 0, true) ) {
      if(XML_GetErrorCode(p) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT) return true;
      if(XML_GetErrorCode(p) == XML_ERROR_FINISHED && iswhitespace(data, size)) return true;
      if(XML_GetErrorCode(p) == XML_ERROR_UNCLOSED_TOKEN) return true;
      parseError(data, 0, XML_ErrorString(XML_GetErrorCode(p)), (int)XML_GetCurrentLineNumber(p));
      return false;
    }
  }

  //  if(done) DEBUG(sax, "Got END_OF_DOCUMENT [%s] at %ld\n", outertag.c_str(), XML_GetCurrentByteIndex(p));

  return done;
}

void Pentominos::SAXParser::parseError(const char *buf, size_t len, std::string error, int lineno)
{
  fprintf(stderr, "SAXParser error at line %d: %s\n", lineno, error.c_str());
  fprintf(stderr, "\tBuffer %u bytes: [", (int)len);
  if(fwrite(buf, len, 1, stderr) != len) {}
  fprintf(stderr, "]\n");
  fflush(stderr);
}

unsigned int Pentominos::SAXParser::usedBytes()
{
  return bufferbytes + (XML_GetCurrentByteIndex(p) - totalbytes);
}

long Pentominos::SAXParser::getBytePos()
{
  return XML_GetCurrentByteIndex(p);
}


#ifdef TEST_SAXPARSER
//deps: log.cc debug.cc
//cflags: -I..
//libs: -lexpat
#include <test.h>

#define XMLFILE "/tmp/saxparsertest.xml"

#include <exception>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

static char xml[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <commit version=\"\" macro=\"referral\" template=\"amd_forunders\">\n"
"  <field value=\"Some docs\" name=\"referral.doctor\"/>\n"
"  <field value=\"DIMS\" name=\"referral.diagnosecode\"/>\n"
"  <field value=\"Avs\" name=\"referral.diagnose\"/>\n"
" </commit>\n"
"</pracro>\n         \t\n\r"
  ;

static char xml_notrailingwhitespace[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user=\"testuser\" cpr=\"0000000000\">\n"
" <commit version=\"\" macro=\"referral\" template=\"amd_forunders\">\n"
"  <field value=\"Some docs\" name=\"referral.doctor\"/>\n"
"  <field value=\"DIMS\" name=\"referral.diagnosecode\"/>\n"
"  <field value=\"Avs\" name=\"referral.diagnose\"/>\n"
" </commit>\n"
"</pracro>"
  ;

static char xml_fail[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user\"testuser\" cpr=\"0000000000\">\n"
" <request macro=\"test\" template=\"test\"/>\n"
"</pracro>\n"
  ;

static char xml_fail2[] =
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<pracro version=\"1.0\" user\"testuser\" cpr=\"0000000000\">\n"
" <request macro=\"test\" template=\"test\"/>\n"
"</pracro>\n"
"this is junk\n"
  ;

class MyFileParser :public Pentominos::SAXParser {
public:
  MyFileParser(const char *file) {
    fd = open(file, O_RDONLY);
  }

  int readData(char *data, size_t size) {
    return read(fd, data, size);
  }

  void startTag(std::string name, std::map< std::string, std::string> attributes)
  {
    //printf("<%s>\n", name.c_str());
  }

  void parseError(const char *buf, size_t len, std::string error, int lineno)
  {
    throw std::exception();
  }

private:
  int fd;
};

class MyBufferParser :public Pentominos::SAXParser {
public:
  void startTag(std::string name, std::map< std::string, std::string> attributes)
  {
    //printf("<%s>\n", name.c_str());
  }

  void parseError(char *buf, size_t len, std::string error, int lineno)
  {
    throw std::exception();
  }
};

TEST_BEGIN;

FILE *fp = fopen(XMLFILE, "w");
TEST_NOTEQUAL(fp, NULL, "Test if file \""XMLFILE"\" could be written.");
if(!fp) TEST_FATAL("Could not write "XMLFILE);
fprintf(fp, "%s", xml);
fclose(fp);

TEST_MSG("Test callback parser.");
{
  MyFileParser parser(XMLFILE);
  parser.parse();
}

TEST_MSG("Test buffer parser.");
for(size_t sz = 1; sz < 1000; sz++) {
  bool test = false;
  MyBufferParser parser;
  std::string buf = xml;
  size_t pos = 0;
  while(pos < buf.length()) {
    std::string substr = buf.substr(pos, sz);
    
    try {
      test |= parser.parse((char*)substr.c_str(), substr.length());
    } catch(std::exception &e) {
      TEST_TRUE(true, "Buffer parser failed on size %d: %s [%s]",
                sz, e.what(), substr.c_str());
    }
    pos += sz;
  }
  
  TEST_TRUE(test, "Test buffer parser on %d bytes", sz);
 }

fp = fopen(XMLFILE, "w");
TEST_NOTEQUAL(fp, NULL, "Test if file \""XMLFILE"\" could be written.");
if(!fp) TEST_FATAL("Could not write "XMLFILE);
fprintf(fp, "%s", xml_notrailingwhitespace);
fprintf(fp, "%s", xml_notrailingwhitespace);
fclose(fp);

TEST_MSG("Test buffer parser with multiple documents in the same buffer.");
{
  fp = fopen(XMLFILE, "r");
  TEST_NOTEQUAL(fp, NULL, "Test if file \""XMLFILE"\" could be read.");
  if(!fp) TEST_FATAL("Could not read from "XMLFILE);

  for(size_t sz = 1; sz < 1000; sz++) {
    MyBufferParser *parser = NULL;
    rewind(fp);
    size_t numdocs = 0;
    char *buf = new char[sz + 1];
    memset(buf, 0, sz + 1);
    size_t size;
    while( (size = fread(buf, 1, sz, fp)) > 0) {
      while(size) {
        if(parser == NULL) {
          parser = new MyBufferParser();
        }
        if(parser->parse(buf, size)) {
          
          // Got one
          numdocs++;
          
          size = size - parser->usedBytes();
          strcpy(buf, buf + parser->usedBytes());
          delete parser; parser = NULL;
        } else {
          size = 0;
          memset(buf, 0, sz + 1);
        }
      }
    }
    TEST_EQUAL(numdocs, 2, "Test if 2 documents were parsed on docsize %d.", sz);
    if(parser) delete parser; parser = NULL;
    delete[] buf;
  }
  fclose(fp);
}

fp = fopen(XMLFILE, "w");
TEST_NOTEQUAL(fp, NULL, "Test if file \""XMLFILE"\" could be written.");
if(!fp) TEST_FATAL("Could not write "XMLFILE);
fprintf(fp, "%s", xml_fail);
fclose(fp);

TEST_MSG("Test failure");
{
  MyFileParser parser(XMLFILE);
  try {
    parser.parse();
  } catch(std::exception &e) {
    goto goon;
  }
  TEST_TRUE(false, "This test should fail...\n");
}
goon:

fp = fopen(XMLFILE, "w");
TEST_NOTEQUAL(fp, NULL, "Test if file \""XMLFILE"\" could be written.");
if(!fp) TEST_FATAL("Could not write "XMLFILE);
fprintf(fp, "%s", xml_fail2);
fclose(fp);

// Test failure
{
  MyFileParser parser(XMLFILE);
  try {
    parser.parse();
  } catch(std::exception &e) {
    goto goonagain;
  }
  TEST_TRUE(false, "This test should fail...\n");
}
goonagain:

unlink(XMLFILE);

TEST_END;

#endif/*TEST_SAXPARSER*/
