/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            uid.cc
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
#include "uid.h"

#include <hugin.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For getpid
#include <unistd.h>
#include <sys/types.h>

// For time
#include <time.h>

// For strerror and errno
#include <errno.h>

// For socket and friends
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <net/if.h>
//#include <netinet/in.h>

// For ioctl
//#include <sys/ioctl.h>

// For random
#include <random>

typedef struct {
  unsigned int rand;
  //  in_addr_t ip;
  time_t time;
  pid_t pid;
  unsigned short int count;
} UID;


//#define SIOCGIFCONF 0x8912 // get iface list

/*
static in_addr_t getIP(const char *interface)
{
  in_addr_t ret = 0;
  int numreqs = 30, sd, n;
  struct ifconf ifc;
  struct ifreq *ifr;
  struct in_addr *ia;

  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd == -1) {
    throw Pentominos::UIDCouldNotConnectException(strerror(errno));
  }

  ifc.ifc_buf = NULL;
  ifc.ifc_len = sizeof(struct ifreq) * numreqs;

  ifc.ifc_buf = (char*)malloc(ifc.ifc_len);
  if(ifc.ifc_buf == NULL) {
    throw Pentominos::UIDOutOfMemoryException();
  }

  if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) {
    throw Pentominos::UIDInterfaceListException(strerror(errno));
  } 

  ifr = ifc.ifc_req; 
  for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq)) {
    ia = (struct in_addr *)((ifr->ifr_ifru.ifru_addr.sa_data)+2);
    if(!strcmp(ifr->ifr_ifrn.ifrn_name, interface)) {
      ret = *(in_addr_t*)ia;
    }
    ifr++; 
  }

  if(!ret) { // Still no interface... We're fucked!
    throw Pentominos::UIDInterfaceException(interface);
  }

  free(ifc.ifc_buf);
  return ret;
}
*/

static unsigned int getRandom()
{
  std::random_device rd;
  return rd();
}

static unsigned short counter = 0;
static unsigned short getCounter()
{
  return counter++;
}

static UID uid = {0,0,0,0};
std::string Pentominos::getUID()
{
  //  if(!uid.ip) uid.ip = getIP(interface);
  if(!uid.rand) uid.rand = getRandom();
  time_t t = time(NULL);
  if(uid.time != t) counter = 0; // If time differes, reset the counter
  uid.time = t; // We need this value every time.

  if(!uid.pid) uid.pid = getpid();

  uid.count = getCounter();

  char buf[32];
  sprintf(buf, "%08x%08x%04x%04x", uid.rand, (unsigned int)uid.time, uid.pid, uid.count);
  return std::string(buf);
}


#ifdef TEST_UID
/**
 * Compile: c++ -DTEST_UID uid.cc exception.cc -o test_uid
 */
int main(int argc, char *argv[])
{
  try {
    printf("New UID: %s\n", Pentominos::getUID("nosuchdevice1").c_str());
  } catch( Pentominos::UIDInterfaceException &e ) {
    fprintf(stderr, "Error: %s\n", e.what());
  } catch( Pentominos::Exception &e ) {
    fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  
  for(int i = 0; i < 10; i++) {
    try {
      printf("New UID: %s\n", Pentominos::getUID().c_str());
    } catch( Pentominos::Exception &e ) {
      fprintf(stderr, "Error: %s\n", e.what());
      return 1;
    }
  }

  for(int i = 0; i < 10; i++) {
    try {
      printf("New UID: %s\n", Pentominos::getUID("lo").c_str());
    } catch( Pentominos::Exception &e ) {
      fprintf(stderr, "Error: %s\n", e.what());
      return 1;
    }
  }

  return 0;
}

#endif/*TEST_UID*/
