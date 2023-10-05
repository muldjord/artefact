/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            oftdevproxy.cc
 *
 *  Wed Mar  3 08:04:11 CET 2010
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
#include <iostream>
#include <string>
#include <vector>

#include <time.h>
#include <string.h>
#include <errno.h>

#include <sysconfig.h>
#include <stdio.h>

#include <unistd.h>

#include <hugin.hpp>

#include <time.h>

#define TIMEOUT 5

#include "artefact.h"

#include "requestparser.h"

#define SEP "\r\n"

long long getCPUTick() {
  long long s64Ret;
  __asm__ __volatile__ ("rdtsc" : "=A"(s64Ret):);
  return s64Ret;
}

int main(int argc, char *argv[])
{
  hug_status_t status;
  status = hug_init(HUG_FLAG_OUTPUT_TO_SYSLOG, 
                    HUG_OPTION_SYSLOG_HOST, "server.dns.addr",
                    HUG_OPTION_SYSLOG_PORT, 514,
                    HUG_OPTION_END);

  if(status != HUG_STATUS_OK) exit(1);

  double mult;
  {
    double len = 10000;
    long long start = getCPUTick();
    usleep(len);
    long long end = getCPUTick();
    mult = (end - start) / len;
  }
  DEBUG(timing, "1 usec == %f ticks\n", mult);

  std::string requestxml;
  std::string replyxml;

  std::string atfhost = "server.dns.addr";
  unsigned short int atfport = 11108;

  DEBUG(recv, "Receiving data...\n");
  long long recv_start = getCPUTick();

  bool done = false;
  time_t start = time(NULL);
  while(!done) {
    DEBUG(recv, "read\n");

    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == -1) {
      ERR(recv, "select() error: %s\n", strerror(errno));
      std::cout << "Read error in select()\n";
      return 1;
    } else if(retval) {
      char buf[4096];
      size_t sz = read(0, buf, sizeof(buf));
      if(sz > 0) {
        requestxml.append(buf, sz);
        RequestParser requestparser_;
        done = requestparser_.parse(requestxml.c_str(), requestxml.length());
      }
    } else {
      // Timeout, try again.
    }

    if((time(NULL) - start) >= TIMEOUT) done = true;
  }

  long long recv_end = getCPUTick();
  DEBUG(recv, "Receiving data - done [%f usec]\n",
        (recv_end - recv_start)/mult);

  DEBUG(request, "Received %zd bytes: [%s]\n",
        requestxml.length(), requestxml.c_str());

  DEBUG(parse, "Parsing data...\n");
  long long parse_start = getCPUTick();

  RequestParser requestparser;
  if(!requestparser.parse(requestxml.c_str(), requestxml.length())) {
    ERR(recv, "XML parser error\n");
    std::cout << "XML error\n";
    return 1;
  }

  long long parse_end = getCPUTick();
  DEBUG(parse, "Parsing data - done [%f usec]\n",
        (parse_end - parse_start)/mult);

  Request request = requestparser.request;

  DEBUG(artefact, "Asking artefact server for results...\n");
  long long atf_start = getCPUTick();

  Artefact artefact(atfhost, atfport);
  replyxml = artefact.get(request);

  long long atf_end = getCPUTick();
  DEBUG(artefact, "Asking artefact server for results - done [%f usec (%f sec)]\n",
        (atf_end - atf_start)/mult, (atf_end - atf_start)/mult/1000000);

  std::cout << replyxml;

  DEBUG(reply, "Sending reply %zd bytes: [%s]\n",
        replyxml.length(), replyxml.c_str());

  hug_close();

  return 0;
}

#ifdef TEST_OFTDEVPROXY
//deps:
//cflags:
//libs:
#include "test.h"

TEST_BEGIN;

TEST_END;

#endif/*TEST_OFTDEVPROXY*/
