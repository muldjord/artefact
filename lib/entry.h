/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            entry.h
 *
 *  Tue Oct 24 22:19:01 CEST 2006
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
#ifndef __ARTEFACT_ENTRY_H__
#define __ARTEFACT_ENTRY_H__

#include <string>
#include <vector>

namespace Pentominos {

class Entry;

/**
 * Data specific values. This is the in-memory representation of a data transaction.
 */
class Data {
public:
  unsigned int id;
  std::string operatorid;
  std::string device_type;
  std::string device_id;
  time_t timestamp;
  std::string location;
  std::string encoding;

  unsigned int length;
  std::string path;
  std::string filename;
  std::vector< std::string > classlist;

  /**
   * Prints the contents of this data object. Used primarily for debugging purposes.
   */
  void print();

  bool operator<(const Data &other) const;

  // For queryhandler:
  std::string uid; // from database
  std::string _class;
};

/**
 * Query specific values. This is the in-memory representation of a query transaction.
 */
typedef enum {
  QF_ALL = 0,
  QF_LATEST = 1
} queryfilter_t;

class Query {
public:
  unsigned int id;
  std::string cls;
  time_t from_time;
  bool from_time_use;
  time_t to_time;
  bool to_time_use;
  queryfilter_t filter;

  /**
   * Prints the contents of this query. Used primarily for debugging purposes.
   */
  void print();
};

/**
 * Entry is the in-memory representation of a xmldatatransfer or xmlquerytransfer.
 */
class Entry {
public:
  ~Entry();

  /**
   * Sets the patientid property. Removes '-' if existing.
   */
  void setPatientID(std::string patientid);

  std::string patientid;
  time_t timestamp;
  bool pretend;

  std::vector< Query * > querylist;
  std::vector< Data * > datalist;

  /**
   * Prints the contents of this entry. Used primarily for debugging purposes.
   */
  void print(const char *prefix = "Entry:");

  /**
   * Checks if the various fields are filled out properly.
   * @return A boolean value, true if the Entry contains only valid fields, false if not.
   */
  bool isvalid();
};

};

#endif/*__ARTEFACT_ENTRY_H__*/
