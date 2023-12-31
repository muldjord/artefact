/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            log.cc
 *
 *  Tue Oct 24 17:44:47 CEST 2006
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
#include "log.h"

#include <hugin.hpp>
#include "tostring.h"

#include <syslog.h>

void Pentominos::log(std::string message)
{
//  fprintf(stderr, message.append("\n").c_str());
  syslog(LOG_CONS, // Write to console if error sending to system logger.
         "%s", message.c_str());
}

// TODO: Remove
/*
void Pentominos::log(std::string filename, int line, std::string message) {
  log(message + " (in " + filename + ":" + toString(line) + ")");
}
*/
/*
void Pentominos::log(char* message, char* file, int line) {
  syslog(LOG_CONS, // Write to console if error sending to system logger.
         message.c_str());
}
*/
