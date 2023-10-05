/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tostring.cc
 *
 *  Thu Mar 29 13:16:13 CEST 2007
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
#include "tostring.h"

#include <hugin.hpp>

#include <stdio.h>

std::string Pentominos::toString(std::string s)
{
  return s;
}

std::string Pentominos::toString(char c)
{
  char buf[32];
  sprintf(buf, "%c", c);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(unsigned char c)
{
  char buf[32];
  sprintf(buf, "%c", c);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(short int si)
{
  char buf[32];
  sprintf(buf, "%d", si);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(short unsigned int su)
{
  char buf[32];
  sprintf(buf, "%u", su);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(int li)
{
  char buf[32];
  sprintf(buf, "%ld", (long int)li);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(unsigned int lu)
{
  char buf[32];
  sprintf(buf, "%lu", (long unsigned int)lu);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(bool b)
{
  if(b) return "true";
  else return "false";
}

std::string Pentominos::toString(float f, unsigned int precision)
{
  char buf[100];
  char format[12];
  sprintf(format, "%%.%uf", precision);
  sprintf(buf, format, f);
  std::string out = buf;
  return buf;
  return "";
}

std::string Pentominos::toString(double d, unsigned int precision)
{
  char buf[100];
  char format[12];
  sprintf(format, "%%.%uf", precision);
  sprintf(buf, format, d);
  std::string out = buf;
  return buf;
}

std::string Pentominos::toString(long double ld, unsigned int precision)
{
  char buf[100];
  char format[12];
  sprintf(format, "%%.%uLg", precision);
  sprintf(buf, format, ld);
  std::string out = buf;
  return buf;
}

#ifdef TEST_TOSTRING
/* Compile:
 * c++ -DTEST_TOSTRING tostring.cc -o test_tostring
 */
using namespace Pentominos;

int main()
{
  std::string s = "string";
  char c = -4;
  unsigned char uc = 'a';
  short int si = 0x8000;
  short unsigned int su = 0xffff;
  //  long int li = 0x80000000L;
  //  long unsigned int lu = 0xffffffffL;
  bool b = true;
  float f = 0.1;
  double d  = 0.1;
  long double ld = 0.1;

std::string str =
  "[" + Pentominos::toString(s)
  + "] [" + Pentominos::toString(c)
  + "] [" + Pentominos::toString(uc)
  + "] [" + Pentominos::toString(si)
  + "] [" + Pentominos::toString(su)
  //  + "] [" + Pentominos::toString(li)
  //  + "] [" + Pentominos::toString(lu)
  + "] [" + Pentominos::toString(b)
  + "] [" + Pentominos::toString(f, 10)
  + "] [" + Pentominos::toString(d, 18)
  + "] [" + Pentominos::toString(ld, 36)
  + "]";

 printf("%s\n", str.c_str()); 

 return 0;
}
#endif/*TEST_TOSTRING*/
