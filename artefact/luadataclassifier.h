/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            luadataclassifier.h
 *
 *  Thu Feb 26 09:55:23 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#ifndef __PENTOMINOS_LUADATACLASSIFIER_H__
#define __PENTOMINOS_LUADATACLASSIFIER_H__

#include <vector>
#include <string>

#include <entry.h>
#include <lua.h>

#include <exception.h>

class LUADataClassifierException : public Pentominos::Exception {
public:
  LUADataClassifierException(std::string what) :
    Pentominos::Exception(what) {}
};

typedef std::string Class;
typedef std::vector< Class > ClassList;

class LUADataClassifier : public Pentominos::LUA
{
public:
  LUADataClassifier();
  ~LUADataClassifier();

  ClassList classify(Pentominos::Data &data);

  void addClass(Class c);
  void error(std::string message);

private:
  ClassList classlist;
};

#endif/*__PENTOMINOS_LUADATACLASSIFIER_H__*/
