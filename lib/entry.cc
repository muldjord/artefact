/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            entry.cc
 *
 *  Tue Oct 24 22:19:02 CEST 2006
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
#include "entry.h"

#include <hugin.hpp>

#include "utf8.h"

Pentominos::Entry::~Entry()
{
  {
    std::vector< Data * >::iterator i = datalist.begin();
    while(i != datalist.end()) {
      delete *i;
      i++;
    }
  }

  {
    std::vector< Query * >::iterator i = querylist.begin();
    while(i != querylist.end()) {
      delete *i;
      i++;
    }
  }
}

void Pentominos::Entry::print(const char *prefix)
{
  UTF8 utf8;

  INFO(entry,"%s\n", prefix);
  INFO(entry,"\tPatientID: %s\n", patientid.c_str());
  DEBUG(entry,"\tTimestamp: %lu\n", timestamp);

  {
    std::vector< Data * >::iterator i = datalist.begin();
    while(i != datalist.end()) {
      (*i)->print();
      i++;
    }
  }

  {
    std::vector< Query * >::iterator i = querylist.begin();
    while(i != querylist.end()) {
      (*i)->print();
      i++;
    }
  }
}

bool Pentominos::Entry::isvalid()
{
  return
    true
    //    patientid.length() == 10
    // Should the program still be running in the year 3000, please change this line
#if 0 // Removed from the spec.
    && birthyear > 0 && birthyear < 3000
    && birthmonth > 0 && birthmonth < 13
    && birthday > 0 && birthday < 32
#endif/*0*/// Removed from the spec.
    && timestamp > 1100000000
    //&& uid.length() == 24
#if 0 // Removed from the spec.
    && givenname != ""
    && surname != ""
#endif/*0*/// Removed from the spec.
    ;
}

void Pentominos::Entry::setPatientID(std::string pid)
{
  // Remove potential '-'
  if(pid.length() == 11 && pid[6] == '-')
    pid = pid.substr(0,6) + pid.substr(7,4);
  patientid = pid;
}

void Pentominos::Data::print()
{
  DEBUG(data, "\tData:\n");
  DEBUG(data, "\tOperator: %s\n", operatorid.c_str());
  DEBUG(data, "\t\tDeviceType: %s\n", device_type.c_str());
  DEBUG(data, "\t\tDeviceID: %s\n", device_id.c_str());
  DEBUG(data, "\t\tTimestamp: %lu\n", timestamp);
  DEBUG(data, "\t\tLocation: %s\n", location.c_str());
  DEBUG(data, "\t\tFilename: %s - %d bytes\n", filename.c_str(), length);
}

bool Pentominos::Data::operator<(const Data &other) const
{
  return timestamp < other.timestamp;
}

void Pentominos::Query::print()
{
  DEBUG(query, "\tQuery:\n");
  //  DEBUG(query, "\t\tFormat: %s\n", format.c_str());
  DEBUG(query, "\t\tClass: %s\n", cls.c_str());
  DEBUG(query, "\t\tFilter: %d\n", filter);
  DEBUG(query, "\t\tFrom time (%s): %lu\n",
                   from_time_use?"use":"nouse", from_time);
  DEBUG(query, "\t\tTo time (%s): %lu\n",
                   to_time_use?"use":"nouse", to_time);

}

#ifdef TEST_ENTRY
//deps: debug.cc log.cc utf8.cc exception.cc
//cflags: -I..
//libs:
#include <test.h>

TEST_BEGIN;

Pentominos::Entry e;
e.setPatientID("000000-0000");
TEST_EQUAL_STR(e.patientid, "0000000000", "Test set");

e.setPatientID("0000000000");
TEST_EQUAL_STR(e.patientid, "0000000000", "Test set");

e.setPatientID("000000000");
TEST_EQUAL_STR(e.patientid, "000000000", "Test set");

e.setPatientID("000000-000");
TEST_EQUAL_STR(e.patientid, "000000-000", "Test set");

TEST_END;

#endif/*TEST_MUTEX*/
