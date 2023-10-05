/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            artefact.cc
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
#include "artefact.h"

Artefact::Artefact(std::string addr, unsigned short port)
{
  handle = atf_init(); // if NULL := "out of memory"

  bool use_https = false;
  conn = atf_connect(handle, addr.c_str(), port, use_https);
}

Artefact::~Artefact()
{
  atf_disconnect(conn);
  atf_close(handle);
}

std::string Artefact::get(Request &request)
{
  std::string replyxml;

  atf_transaction_t* trans =
    atf_new_transaction(conn, request.patientid.c_str());

  int timeflags = USE_NONE;
  if(request.has_start) timeflags |= USE_FROM_TIME;
  if(request.has_end) timeflags |= USE_TO_TIME;

  std::vector<atf_id> ids;
  std::vector<std::string>::iterator i = request.categories.begin();
  while(i != request.categories.end()) {
    atf_id id = atf_add_query(trans, i->c_str(),
                              FILTER_ALL,
                              timeflags,
                              request.start, request.end);
    ids.push_back(id);
    i++;
  }

  atf_reply_t *reply = atf_commit(trans);

  std::vector<atf_id>::iterator j = ids.begin();
  while(j != ids.end()) {
    for(size_t idx = 0; idx < atf_get_num_results(reply, *j); idx++) {
      atf_result_t *result = atf_get_result(reply, *j, idx);
      atf_status_t status  = atf_get_result_status(result, NULL, 0);
      if(status) {/*hmm*/}
    }
    j++;
  }

  size_t bufsz = atf_get_reply_raw_size(reply);
  char *buf = new char[bufsz];
  atf_get_reply_raw(reply, buf, bufsz);
    
  replyxml.append(buf, bufsz);
    
  delete[] buf;

  atf_free_reply(reply);
  atf_free_transaction(trans);

  return replyxml;
}

#ifdef TEST_ARTEFACT
//deps:
//cflags:
//libs:
#include "test.h"

TEST_BEGIN;

TEST_END;

#endif/*TEST_ARTEFACT*/
