/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            mkdirr.h
 *
 *  Mon Mar 19 14:30:29 CET 2007
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
#ifndef __ARTEFACT_MKDIRR_H__
#define __ARTEFACT_MKDIRR_H__

#include <string>

#include "exception.h"

namespace Pentominos {

  class MkdirRDirectoryException: public Pentominos::Exception {
    public:
    MkdirRDirectoryException(std::string dir, std::string reason) : 
      Pentominos::Exception("Could not create directory " + dir + " in mkdirr: " + reason) {}
  };

  class MkdirRStatException: public Pentominos::Exception {
    public:
    MkdirRStatException(std::string dir, std::string reason) : 
      Pentominos::Exception("Could not stat directory " + dir + " in mkdirr: " + reason) {}
  };

  /**
   * Make Dir Recursive.\n
   * Parses a path string and checks every directory in it. If the directory does
   * not exist, it is created.\n
   * This function might throw the MkdirRException exception.
   * @param path An STL string containing 
   */
  int mkdirr(std::string path);
  
  /**
   * Strip actual filename from a full path.
   * @param filename An STL string containing the full path and filename to be stripped.
   */
  std::string stripFilename(std::string filename);

};

#endif/*__ARTEFACT_MKDIRR_H__*/
