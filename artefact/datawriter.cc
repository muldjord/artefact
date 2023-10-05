/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            datawriter.cc
 *
 *  Fri Mar 23 10:00:04 CET 2007
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
#include "datawriter.h"

#include <hugin.hpp>

// For mkstemp
#include <stdlib.h>

#include <string.h>
#include <errno.h>
#include <config.h>
#include <mkdirr.h>
#include <unistd.h>

// For open/close/write
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <tostring.h>

DataWriter::DataWriter(Pentominos::Data *data)
{
  this->data = data;
  data->length = 0;

  // The last six characters of template must be XXXXXX and these are
  // replaced with a string that makes the filename unique.
  char stemplate[] = "/tmp/artefact-XXXXXX";
  this->fd = mkstemp(stemplate);

  if(this->fd == -1) throw DataWriterFileException(stemplate, strerror(errno));

  // Here the filename is absolute.
  this->data->filename = stemplate;

}

DataWriter::~DataWriter()
{
  if(this->fd != -1) close(this->fd);
}

void DataWriter::append(const char *buf, size_t size)
{
  if(this->fd != -1) {
    int written = write(fd, buf, size);
    if(written < (int)size)
      throw DataWriterFileException(data->filename, strerror(errno));
    data->length += size;
  }
}

#ifdef TEST_DATAWRITER

int main()
{
  char buf[] = "hello world";
  Pentominos::Data data;

  try {
    DataWriter writer(&data);

    int i = 10;
    while(i) {
      writer.append(buf, sizeof(buf));
      i--;
    }

  } catch( Pentominos::Exception &e ) {
    ERR(datawriter, "Error: %s\n", e.what());
    unlink(data.filename.c_str());
    return 1;
  }
 
  if(data.length != 10 * sizeof(buf)) {
    ERR(datawriter, "Datasize mismatch!\n");
    return 1;
  }

  if(unlink(data.filename.c_str()) == -1) {
    ERR(datawriter, "Temporary data file was not created!\n");
    return 1;
  }

  return 0;
}

#endif/*TEST_DATAWRITER*/
