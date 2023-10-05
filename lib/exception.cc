/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            exception.cc
 *
 *  Mon Mar 19 11:33:15 CET 2007
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
#include "exception.h"

#include <hugin.hpp>

#include "log.h"

Pentominos::Exception::Exception(std::string what, Pentominos::ExceptionLevel level)
{
  std::string prefix;
  switch(level) {
  case LEVEL_WARNING:
    prefix = "Warning: ";
    break;

  case LEVEL_ERROR:
    prefix = "Error: ";
    break;
  }

  this->_what = prefix + what;
  this->level = level;

  Pentominos::log(_what);
}

const char* Pentominos::Exception::what() const throw()
{
  return _what.c_str();
}


#ifdef TEST_EXCEPTION

class MyException : public Pentominos::Exception {
public:
  MyException() :
    Pentominos::Exception("MyException has been thrown") {}
};

class MyExtException : public Pentominos::Exception {
public:
  MyExtException(std::string thingy) :
    Pentominos::Exception("MyExtException has been thrown: " + thingy) {}
};

int main()
{
  try {
    throw MyException();
  } catch( MyException &e ) {
    printf("%s\n", e.what());
  }

  try {
    throw MyExtException("Yeaaah!");
  } catch( MyExtException &e ) {
    printf("%s\n", e.what());
  }

  return 0;
}

#endif/*TEST_EXCEPTION*/
