/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            queryhandler.h
 *
 *  Wed Apr  4 09:18:46 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
#ifndef __ARTEFACT_QUERYHANDLER_H__
#define __ARTEFACT_QUERYHANDLER_H__

#include "entry.h"
#include "postgresql.h"

#include "exception.h"

#include "tree.h"

/**
 * Query Handler Exceptions
 */
class QueryHandlerException : public Pentominos::Exception {
public:
	QueryHandlerException(std::string reason)
    : Pentominos::Exception("Error in queryhandler: " + reason) {}
};

class QueryHandler {
public:
  QueryHandler(Pentominos::Entry &entry,
               PostgreSQLTransaction &pgsql,
               bool resume);

  Trees handle(Pentominos::Query &query);

private:
  Pentominos::Entry &entry;
  PostgreSQLTransaction &pgsql;
  bool resume;
};

#endif/*__ARTEFACT_QUERYHANDLER_H__*/
