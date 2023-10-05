/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            environment.cc
 *
 *  Wed Feb 10 15:50:18 CET 2010
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
#include "environment.h"

#include "postgresql.h"

#include <hugin.hpp>

Environment::Environment()
{
  int database_poolsize = 1;
  for(int i = 0; i < database_poolsize; i++) {
    dbpool.add(new PostgreSQL("localhost",
                              5432, "artefact",
                              "artefact", "artefact"));
  }
}

Environment::~Environment()
{
}

#ifdef TEST_ENVIRONMENT
//deps: postgresql.cc connectionpool.cc ../lib/mutex.cc ../lib/semaphore.cc ../lib/debug.cc ../lib/uid.cc ../lib/exception.cc ../lib/log.cc ../lib/status.cc
//cflags: -I../lib -I.. $(PQXX_CXXFLAGS) $(PTHREAD_CFLAGS)
//libs: $(PQXX_LIBS) -lpthread $(PTHREAD_LIBS)
#include "test.h"

TEST_BEGIN;

TEST_NOEXCEPTION(Environment env, "Check if the Enviroment can be created.");

TEST_END;

#endif/*TEST_ENVIRONMENT*/
