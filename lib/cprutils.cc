/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            cprutils.cc
 *
 *  Thu Apr 24 12:18:33 CEST 2008
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
#include "cprutils.h"

#include <hugin.hpp>

#include <time.h>
#include <stdlib.h>

int cprtoyear(std::string &cpr)
{
  if(cpr.length() < 10) return 0;

  // Remove potential '-'
  if(cpr[6] == '-') cpr = cpr.substr(0,6) + cpr.substr(7,4);

  int year = atoi(cpr.substr(4,2).c_str());
  int control = atoi(cpr.substr(6,4).c_str());

  if(year >=  0 && year <= 99 && control >= 1    && control <=  999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 1000 && control <= 1999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 2000 && control <= 2999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 3000 && control <= 3999) return year + 1900;
  if(year >=  0 && year <= 36 && control >= 4000 && control <= 4999) return year + 2000;
  if(year >= 37 && year <= 99 && control >= 4000 && control <= 4999) return year + 1900;
  if(year >=  0 && year <= 36 && control >= 5000 && control <= 5999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 5000 && control <= 5999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 6000 && control <= 6999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 6000 && control <= 6999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 7000 && control <= 7999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 7000 && control <= 7999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 8000 && control <= 8999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 8000 && control <= 8999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 9000 && control <= 9999) return year + 2000;
  if(year >= 37 && year <= 99 && control >= 9000 && control <= 9999) return year + 1900;
  /*
  //  int year = atoi(cpr.substr(4, 2).c_str());
  time_t t = time(NULL);
  struct tm *_tm = localtime(&t);
  int nowyear = _tm->tm_year + 1900;
  int decade = nowyear / 100 * 100;

  if(year > nowyear % 100) year += decade - 100;
  else year += decade;
  */
  return year;
}

int cprtomonth(std::string &cpr)
{
  if(cpr.length() < 10) return 0;
  int month = atoi(cpr.substr(2, 2).c_str());
  return month;
}

int cprtoday(std::string &cpr)
{
  if(cpr.length() < 10) return 0;
  int day = atoi(cpr.substr(0, 2).c_str());
  return day;
}


#ifdef TEST_CPRUTILS
//deps:
//cflags: -I..
//libs:
#include <test.h>

TEST_BEGIN;
std::string cpr = "0000000000";
TEST_EQUAL_INT(cprtoyear(cpr), 1905, "Test year");
TEST_EQUAL_INT(cprtomonth(cpr), 5, "Test month");
TEST_EQUAL_INT(cprtoday(cpr), 15, "Test day");

cpr = "000000-0000";
TEST_EQUAL_INT(cprtoyear(cpr), 1905, "Test year");
TEST_EQUAL_INT(cprtomonth(cpr), 5, "Test month");
TEST_EQUAL_INT(cprtoday(cpr), 15, "Test day");

TEST_END;

#endif/*TEST_CPRUTILS*/
