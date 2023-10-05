/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            transactionlogger.cc
 *
 *  Thu Mar 29 10:53:20 CEST 2007
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
#include "transactionlogger.h"

#include <hugin.hpp>

#include <time.h>

#include <errno.h>
#include <string.h>

// For open/close/write
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <lib/config.h>
#include <mkdirr.h>

#include "tostring.h"

std::string TransactionLogger::getFilename()
{
  std::string filename;

  char buf[512];

  std::string logformat;
  //  if(Artefact::config()->get("logformat", &logformat)) {
  //    fprintf(stderr, "Could not read logformat.");
  //    logformat = "%u.xml";
  //  }
  try {
    std::string l = Pentominos::config["logformat"];
    logformat = l;
  } catch( ... ) {
    throw TransactionLoggerConfigException("Could not look up logformat.");
  }

  time_t now = time(NULL);
  int i = 0;
  while(i < (int)logformat.size()) {
    if(logformat.at(i) == '%') {
      i++;
      switch(logformat.at(i)) {
      case 'u': // %u - Unixtimestamp of the processingtime. 
        sprintf(buf, "%u", (unsigned int)now);
        break;
      case 's': // %s - Unixtimestamp of the sender.
        sprintf(buf, "%u", (unsigned int)entry.timestamp);
        break;
        /*
      case 'i': // %i - Sender ip address.
        sprintf(buf, "%s", entry.src_addr.c_str());
        break;
      case 'r': // %r - Receiver ip address.
        sprintf(buf, "%s", entry.dst_addr.c_str());
        break;
        */
      case '%': // %% - %
        sprintf(buf, "%%");
        break;
      default:
        sprintf(buf, "%%%c", logformat.at(i)); // Simply write the code.
        break;
      }
    } else {
      sprintf(buf, "%c", logformat.at(i));
    }

    filename.append(buf);
    i++;
  }

  if(counter > 0) {
    filename += "-" + Pentominos::toString(counter);
  }

  return filename;
}

TransactionLogger::TransactionLogger(Pentominos::Entry &e)
  : entry(e)
{
  this->fd = -1;
  this->haserror = false;
}

TransactionLogger::~TransactionLogger()
{
  if(entry.isvalid()) flush();
  //  if(buffer != "") fprintf(stderr, "Discarding transactionbuffer...!\n");
  if(fd > -1) close(fd);
}

static ssize_t _write(int fd, const void *buf, size_t count){return write(fd, buf, count);}
void TransactionLogger::flush()
{
  if(this->haserror) return;

  if(fd == -1) {
    counter = 0;

    std::string logdir;
    try {
      std::string l = Pentominos::config["logdir"];
      logdir = l;
    } catch( ... ) {
      this->haserror = true;
      throw TransactionLoggerConfigException("Could not look up logdir.");
    }

    try {
      Pentominos::mkdirr(logdir);
    } catch( ... ) {
      // ... hmmm hat to do here?
    }

    do {
      this->filename = logdir + "/" + getFilename();
      //      printf("Trying logfile %s\n", this->filename.c_str());
      
      this->fd = open(this->filename.c_str(),
                      O_CREAT | O_WRONLY | O_ASYNC | O_EXCL,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if(errno == EEXIST) counter++;
    } while(this->fd == -1 && errno == EEXIST);
    
    if(this->fd == -1) {
      this->haserror = true;
      throw TransactionLoggerFileException(this->filename, strerror(errno));
    }

    //    printf("Writing logfile %s\n", this->filename.c_str());

  }

  int size = buffer.length();
  if(_write(fd, buffer.c_str(), size) < size) {
    this->haserror = true;
    throw TransactionLoggerFileException(this->filename, strerror(errno));
  }

  buffer = "";
}

void TransactionLogger::write(char *data, size_t size)
{
  buffer.append(data, size);
  if(entry.isvalid()) flush();
}


void TransactionLogger::writeStartTag(std::string &name, std::map< std::string, std::string> attributes)
{
  std::string tag = "</" + name;
  std::map< std::string, std::string>::iterator i = attributes.begin();
  while(i != attributes.end()) {
    tag += " " + i->first + "=\"" + i->second + "\"";
    i++;
  }
  tag += ">\n";
  write((char*)tag.c_str(), tag.size());
}

void TransactionLogger::writeEndTag(std::string &name)
{
  std::string tag = "</" + name + ">\n";
  write((char*)tag.c_str(), tag.size());
}

void TransactionLogger::writeCharacterData(std::string &cdata)
{
  write((char*)cdata.c_str(), cdata.size());
}

#ifdef TEST_TRANSACTIONLOGGER

#define CONFNAME "/tmp/test_transactionlogger.conf"

int main()
{
  Pentominos::Entry entry;
    
  // Fill in the nessecary fields to make the entry valid.
  entry.cpr = "2003791613";
  entry.birthyear = 1979;
  entry.birthmonth = 3;
  entry.birthday = 20;
  entry.timestamp = time(NULL);
  entry.uid = "123456789012345678901234";
  entry.givenname = "Bent Biballe";
  entry.surname = "Nyeng";


  { // Positive test example

    // Create a conf file
    FILE *fp = fopen(CONFNAME,"w");
    fprintf(fp, "logformat=\"test_transactionlogger.xml\"");
    fprintf(fp, "logdir=\"/tmp\"");
    fclose(fp);
    
    // Init the conf thingy
    //    Pentominos::Configuration conf(CONFNAME);
    //    Pentominos::initConfig(&conf);
    Pentominos::parse_config(CONFNAME);
  
    try {
      TransactionLogger logger(&entry);
      
      int i = 10;
      char buf[] = "hello world";
      while(i) {
        logger.write(buf, sizeof(buf));
        i--;
      }
    } catch( Pentominos::Exception &e ) {
      fprintf(stderr, "Error: %s\n", e.what());
      unlink(CONFNAME);
      unlink("/tmp/test_transactionlogger.xml");
      return 1;
    }
    
    unlink(CONFNAME);
    unlink("/tmp/test_transactionlogger.xml");
  }

  { // Negative test example

    // Create a conf file
    FILE *fp = fopen(CONFNAME,"w");
    fprintf(fp, "logformat=\"test_transactionlogger.xml\"");
    fprintf(fp, "logdir=\"/tmp/somedirthatdoesnotexist\"");
    fclose(fp);
    
    // Init the conf thingy
    Pentominos::config.clear();
    Pentominos::parse_config(CONFNAME);

    try {
      TransactionLogger logger(&entry);
    
      int i = 10;
      char buf[] = "hello world";
      while(i) {
        logger.write(buf, sizeof(buf));
        i--;
      }
      return 1;
        
    } catch( Pentominos::Exception &e ) {
      fprintf(stderr, "Error: %s\n", e.what());
      unlink(CONFNAME);
      unlink("/tmp/test_transactionlogger.xml");
    }
      
    unlink(CONFNAME);
    unlink("/tmp/test_transactionlogger.xml");
  }

  return 0;
}

#endif/*TEST_TRANSACTIONLOGGER*/
