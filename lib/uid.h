/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            uid.h
 *
 *  Mon Oct 30 12:52:10 CET 2006
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
#ifndef __ARTEFACT_UID_H__
#define __ARTEFACT_UID_H__

#include <string>
#include "exception.h"

namespace Pentominos {

  /**
   * The UIDOutOfMemoryException is thrown by getUID() when the system runs out of memory.
   */
  class UIDOutOfMemoryException : public Pentominos::Exception {
  public:
    UIDOutOfMemoryException() :
      Pentominos::Exception("Out of memory in UID generation.") {}
  };
  
  /**
   * The UIDInterfaceException is thrown by getUID() if the interface requested cannot be found.
   */
  class UIDInterfaceException : public Pentominos::Exception {
  public:
    UIDInterfaceException(std::string interface) :
      Pentominos::Exception("No such interface in UID generation: " + interface) {}
  };
  
  /**
   * The UIDInterfaceListException is thrown by getUID() if the interfacelist (list of connected
   * network interfaces) cannot be accessed.
   */
  class UIDInterfaceListException : public Pentominos::Exception {
  public:
    UIDInterfaceListException(std::string reason) :
      Pentominos::Exception("Could not get interface list in UID generation: " + reason) {}
  };
  
  /**
   * The UIDCouldNotConnectException is thown by getUID() when the local connection required
   * to get the local ip adress, cannot be established.
   */
  class UIDCouldNotConnectException : public Pentominos::Exception {
  public:
    UIDCouldNotConnectException(std::string reason) :
      Pentominos::Exception("Could not make a connection in UID generation: " + reason) {}
  };
  
  /**
   * getUID produces a unique identifier for the whole system (not just machine
   * local) based on a random number, the process id of the app, current time,
   * and a counter.
   * The UID is 24 bytes long.
   * @return The unique identifier in hex, contained in an STL string.
   */
  std::string getUID();

};

#endif/*__ARTEFACT_UID_H__*/
