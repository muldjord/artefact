/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            artefact.h
 *
 *  Wed Mar  3 08:34:27 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#ifndef __PENTOMINOS_ARTEFACT_H__
#define __PENTOMINOS_ARTEFACT_H__

#include <string>
#include <time.h>
#include <libartefact.h>

#include "request.h"

class Artefact {
public:
  Artefact(std::string addr, unsigned short port);
  ~Artefact();

  std::string get(Request &request);

private:
  atf_handle_t *handle;
  atf_connection_t *conn;
};

#endif/*__PENTOMINOS_ARTEFACT_H__*/
