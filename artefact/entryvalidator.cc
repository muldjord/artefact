/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            entryvalidator.cc
 *
 *  Mon Apr  2 11:14:41 CEST 2007
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
#include "entryvalidator.h"

#include <hugin.hpp>

#include <config.h>
#include <tostring.h>

// For open/close/read/write
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mkdirr.h>

#include <string.h>
#include <errno.h>

// For unlink
#include <unistd.h>

#include <cprutils.h>

#include "luadataclassifier.h"

/*
 * Get a filehandle to a permanent file.
 */
static int permfile(Pentominos::Entry &entry, Pentominos::Data *data)
{
  int fd = -1;
  char buf[512];
  unsigned int counter = 0;

  std::string filename;

  std::string dataformat;

  try {
    std::string d = Pentominos::config["dataformat"];
    dataformat = d;
  } catch( ... ) {
    ERR(entryvalidator, "Could not read 'dataformat' from config file.");
    return -1;
  }


  std::string datapath;
  //  if(Artefact::config()->get("datapath", &datapath)) {
  //    fprintf(stderr, "Could not read datapath.");
  //    return -1;
  //  }
  try {
    std::string d = Pentominos::config["datapath"];
    datapath = d;
  } catch( ... ) {
    ERR(entryvalidator, "Could not read 'datapath' from config file.");
    return -1;
  }

  int i = 0;
  while(i < (int)dataformat.size()) {
    if(dataformat.at(i) == '%') {
      i++;
      switch(dataformat.at(i)) {
      case 'u': // %u - Unixtime of data production.
        sprintf(buf, "%u", (unsigned int)data->timestamp);
        break;

      case 'i': // %i - Patient unique ID.
        sprintf(buf, "%s", entry.patientid.c_str());
        break;

      case 'y': // %y - Patient year of birth.
        sprintf(buf, "%d", cprtoyear(entry.patientid));
        break;

      case 'm': // %m - Patient month of birth.
        sprintf(buf, "%d", cprtomonth(entry.patientid));
        break;

      case 'd': // %d - Patient day of birth.
        sprintf(buf, "%d", cprtoday(entry.patientid));
        break;

      case 't': // %t - Device type.
        sprintf(buf, "%s", data->device_type.c_str());
        break;

      case 'x': // %x - Device id.
        sprintf(buf, "%s", data->device_id.c_str());
        break;

      case '%': // %% - %
        sprintf(buf, "%%");
        break;

      default:
        sprintf(buf, "%%%c", dataformat.at(i)); // Simply write the code.
        break;
      }
    } else {
      sprintf(buf, "%c", dataformat.at(i));
    }

    filename.append(buf);

    i++;
  }

  if(datapath.at(datapath.length() - 1) != '/') datapath.append("/");

  std::string tmpfilename;

  do {
    data->path = datapath;
    data->filename = filename;

    if(counter > 0) {
      data->filename += "-" + Pentominos::toString(counter);
    }

    INFO(entryvalidator, "Trying file %s%s\n",
                    data->path.c_str(), data->filename.c_str());

    tmpfilename = data->path + data->filename;
    try {
      Pentominos::mkdirr(Pentominos::stripFilename(tmpfilename));
    } catch( Pentominos::Exception &e ) {
      ERR(entryvalidator, "%s\n", e.what());
    }

    fd = open(tmpfilename.c_str(),
              O_CREAT | O_WRONLY | O_ASYNC | O_EXCL,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(errno == EEXIST) counter++;
  } while(fd == -1 && errno == EEXIST);
  
  INFO(entryvalidator, "Writing file %s\n", tmpfilename.c_str());

  return fd;
}

EntryValidator::EntryValidator(Pentominos::Entry &e)
  : entry(e)
{
}

bool EntryValidator::validate()
{
  if( entry.isvalid() == false ) return false;

  try {
    LUADataClassifier classifier;

    std::vector< Pentominos::Data * >::iterator i = entry.datalist.begin();
    while(i != entry.datalist.end()) {
      Pentominos::Data *data = *i;
      try {
        data->classlist = classifier.classify(*data);
      } catch(...) {
        return false;
      }
      i++;
    }
  } catch( Pentominos::Exception &e ) {
    ERR(entryvalidator, "LUA Exception: %s.\n", e.what());
    return false;
  }

  return true;
}

void EntryValidator::erase()
{
  std::vector< Pentominos::Data * >::iterator i = entry.datalist.begin();
  while(i != entry.datalist.end()) {
    Pentominos::Data *data = *i;
    WARN(entryvalidator, "Deleting file %s%s\n",
         data->path.c_str(), data->filename.c_str());
    std::string f = data->path + data->filename;
    unlink(f.c_str());
    i++;
  }
}

void EntryValidator::fixate()
{
  std::vector< Pentominos::Data * >::iterator i = entry.datalist.begin();
  while(i != entry.datalist.end()) {
    Pentominos::Data *data = *i;
    std::string tempfile = data->path + data->filename;

    // Duplicate file.
    int tf = open(tempfile.c_str(), O_RDONLY | O_ASYNC);
    int pf = permfile(entry, data); // The data filename property is 
                                    //  overwritten with the new one.
    char buf[1024];
    size_t size;
    while((size = read(tf, buf, sizeof(buf))) > 0) {
      if(write(pf, buf, size) == -1) {
        ERR(entryvalidator, "Could not write to file: %s\n", strerror(errno));
      }
    }
    close(pf);
    close(tf);

    // Delete temp file
    unlink(tempfile.c_str());

    i++;
  }
}

#ifdef TEST_ENTRYVALIDATOR
//deps: ../lib/uid.cc ../lib/exception.cc ../lib/debug.cc ../lib/log.cc ../lib/entry.cc ../lib/utf8.cc ../lib/config.cc luadataclassifier.cc ../lib/lua.cc ../lib/luautil.cc ../lib/cprutils.cc ../lib/tostring.cc ../lib/mkdirr.cc
//cflags: -I../lib -I.. $(LUA_CFLAGS)
//libs:  $(LUA_LIBS)
#include "test.h"

#define CPR "0000000000"

std::string lua_basedir = "/tmp";

TEST_BEGIN;

///////
/////// Test 'pretend' feature in database.
///////

Pentominos::Entry entry;
entry.patientid = CPR;

Pentominos::Data data;

data.classlist.push_back("dims");
data.classlist.push_back("dut");

data.length = 10;
data.timestamp = time(NULL);
data.device_type = "abekat";
data.device_id = "hun";
data.location = "abeburet";
data.filename = "/tmp/guldkaramel";
data.operatorid = "1234";

TEST_END;

#endif/*TEST_ENTRYVALIDATOR*/
