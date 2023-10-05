/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            transactionlogger.h
 *
 *  Thu Mar 29 10:53:20 CEST 2007
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
#ifndef __ARTEFACT_TRANSACTIONLOGGER_H__
#define __ARTEFACT_TRANSACTIONLOGGER_H__

#include "exception.h"

#include "entry.h"

#include <map>
#include <string>

/**
 * TransactionLoggerConfigException.
 */
class TransactionLoggerConfigException : public Pentominos::Exception {
public:
	TransactionLoggerConfigException(std::string reason) :
    Pentominos::Exception("Config error in transaction logger: " + reason) {}
};

/**
 * TransactionLoggerFileException.
 */
class TransactionLoggerFileException : public Pentominos::Exception {
public:
	TransactionLoggerFileException(std::string file, std::string reason) :
    Pentominos::Exception("File error (" + file + ") in transaction logger: " + reason) {}
};

/**
 * This class takes care of the logging of transactions between a client and
 * the server. It writes to a file that is named according to the Entry, and
 * only begins the actual writing when the entry contains valid data.\n
 * When the entry does not contain valid data, the logger buffers the incoming
 * data internally for later writing.
 */
class TransactionLogger {
public:
  /**
   * The contructor.\n
   * @param entry A pointer to the entry that is to be used for naming the log
   * file. It need not be valid at this time.
   */
  TransactionLogger(Pentominos::Entry &entry);

  /**
   * Clean up and flush the logfile.\n
   * Note: If the given entry still doesn't contain valid data, the buffered
   * log data will be discarded.
   */
  ~TransactionLogger();

  /**
   * Use this function to feed the logger.\n
   * This method might throw the following exception:
   * TransactionLoggerConfigException and TransactionLoggerFileException.
   * @param data A pointer to the data to be logged.
   * @param size The size of the data buffer.
   */
  void write(char *data, size_t size);

  void writeStartTag(std::string &name, std::map< std::string, std::string> attributes);
  void writeEndTag(std::string &name);
  void writeCharacterData(std::string &cdata);

private:
  Pentominos::Entry &entry;
  std::string buffer;

  std::string filename;

  void flush();

  unsigned int counter;
  std::string getFilename();
  int fd;

  bool haserror;
};

#endif/*__ARTEFACT_TRANSACTIONLOGGER_H__*/
