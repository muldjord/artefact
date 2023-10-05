/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            datawriter.h
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
#ifndef __ARTEFACT_DATAWRITER_H__
#define __ARTEFACT_DATAWRITER_H__

#include "entry.h"
#include <string>

#include "exception.h"


/**
 * DataWriterFileException.
 */
class DataWriterFileException : public Pentominos::Exception {
public:
  DataWriterFileException(std::string file, std::string reason) :
    Pentominos::Exception("DataWriter failed on " + file + ": " + reason) {}
};


/**
 * This class is used to write data to disk and connect these to a Data
 * object.\n
 * The file used will be created using mkstemp and the filename property of
 * the Data object will be set accordingly.
 */
class DataWriter {
public:
  /**
   * Constructor.\n
   * @param data The data object.
   */
  DataWriter(Pentominos::Data *data);

  /**
   * Destructor.
   */
  ~DataWriter();

  /**
   * The method appends data to the temporary file, and increases the
   * length property of the Data object.\n
   * This method might throw the exception DataWriterFileException.
   * @param data A pointer to the data to be appended.
   * @param size The size of the data to be appended.
   */
  void append(const char *data, size_t size);

private:
  Pentominos::Data *data;

  int fd;
};

#endif/*__ARTEFACT_DATAWRITER_H__*/
