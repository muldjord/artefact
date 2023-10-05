/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tcpsocket.cc
 *
 *  Thu Oct 19 10:24:25 CEST 2006
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
#include "tcpsocket.h"

#include <hugin.hpp>

#include "tostring.h"

// for gethostbyname
#include <netdb.h>

// for inet_addr
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// for connect, listen, bind and accept
#include <sys/types.h>
#include <sys/socket.h>

// For socket
#include <sys/types.h>
#include <sys/socket.h>

// For TCP
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

// For inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For close
#include <unistd.h>

#include <string.h>
#include <errno.h>
#include <stdio.h>

// For ioctl
#include <sys/ioctl.h>

// For socket and friends
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

// For fcntl
#include <unistd.h>
#include <fcntl.h>

// For usleep
#include <unistd.h>

Pentominos::TCPSocket::TCPSocket()
{
  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    throw TCPSocketException(strerror(errno));
  }
  isconnected = false;
}

Pentominos::TCPSocket::~TCPSocket()
{
  disconnect();
}


static int _listen(int sockfd, int backlog){return listen(sockfd, backlog);}
void Pentominos::TCPSocket::listen(unsigned short int port)
{

  if(sock == -1) {
    throw TCPListenException("Socket not initialized.");
  }

  if(isconnected) {
    throw TCPListenException("Socket already connected.");
  }

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(sock, (struct sockaddr*)&socketaddr, sizeof(socketaddr)) == -1) {
    throw TCPListenException(std::string("bind failed - ") + strerror(errno));
  }

  if(_listen(sock, 5) == -1) {
    throw TCPListenException(std::string("listen failed - ") + strerror(errno));
  }

  isconnected = true;
}


static int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{return accept(sockfd, addr, addrlen);}
Pentominos::TCPSocket Pentominos::TCPSocket::accept()
{
  TCPSocket child;

  if(sock == -1) {
    throw TCPAcceptException("Socket not initialized.");
  }

  if(!isconnected) {
    throw TCPAcceptException("Socket not connected.");
  }

  // accept new connection and get its connection descriptor
  struct sockaddr_in ssocketaddr;
  int csalen = sizeof(ssocketaddr);

  child.disconnect(); // We need to close the existing socket
  child.sock = _accept(sock, (struct sockaddr*)&ssocketaddr, (socklen_t*)&csalen);

  if (child.sock == -1) {
    throw TCPAcceptException(std::string("accept failed - ") + strerror(errno));
  }

  child.isconnected = true;
  return child;
}


static int _connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{return connect(sockfd, serv_addr, addrlen);}
void Pentominos::TCPSocket::connect(std::string addr, unsigned short int port)
{
  if(isconnected) {
    throw TCPConnectException("Socket already connected.", "", "");
  }

#ifndef BYPASS_STATICALLOCATIONS
  // Do DNS lookup
  char *ip;
  struct in_addr **addr_list;
  struct hostent *he;
  he = gethostbyname(addr.c_str());
  if(!he || !he->h_length) {
    throw TCPConnectException(addr, toString(port),
                              std::string("host lookup failed: ") + hstrerror(h_errno));
  }

  addr_list = (struct in_addr **)he->h_addr_list;
  //  Get first value. We know for sure that there are at least one.
  ip = inet_ntoa(*addr_list[0]);
#else/*BYPASS_STATICALLOCATIONS*/
  char *ip = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = inet_addr(ip);

  if(_connect(sock, (struct sockaddr*)&socketaddr, sizeof(socketaddr))) {
    throw TCPConnectException(addr, toString(port), hstrerror(h_errno));
  }

  isconnected = true;
}

void Pentominos::TCPSocket::disconnect()
{
  if(sock != -1) {
    close(sock);
    sock = -1;
  }
  isconnected = false;
}

bool Pentominos::TCPSocket::connected()
{
  return sock != -1 && isconnected;
}

ssize_t _read(int fd, void *buf, size_t count) { return read(fd, buf, count); }
int Pentominos::TCPSocket::read(char *buf, size_t size, long timeout)
{
  int res = 0;

  if(sock == -1) throw TCPReadException("Socket not initialized.");
  if(!isconnected) throw TCPReadException("Socket is not connected.");

  // Select
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = timeout;

  struct timeval *ptv = NULL;
  if(timeout >= 0) ptv = &tv;

  fd_set fset;
  int ret;
  FD_ZERO(&fset);
  FD_SET(sock, &fset);
  ret = select (sock+1, &fset, NULL, NULL, ptv);
  switch(ret) {
  case -1:
    if(errno == EINTR) {
      INFO(tcpsocket, "EINTR - got interrupt\n");
      return -1; // a signal caused select to return. That is OK with me
    } else {
      throw TCPReadException("Select on socket (read) failed.");
    }
    break;

  case 0:
    // timeout
    INFO(tcpsocket, "Timeout\n");
    break;

  default:
    if(FD_ISSET(sock, &fset)) {
      //      res = recv(sock, buf, size, MSG_DONTWAIT);
      if( (res = _read(sock, buf, size)) == -1 ) {
        throw TCPReadException(strerror(errno));
      }
    } else {
      INFO(tcpsocket, "FD_ISSET failed (timeout?)\n");
      return 0;
    }
  }

  return res;
}
/*int Pentominos::TCPSocket::read(char *buf, int size)
  throw(TCPReadException)
{
  int res = 0;

  if(sock == -1) {
    throw TCPReadException("Socket not initialized.");
  }

  if(!isconnected) {
    throw TCPReadException("Socket is not connected.");
  }

  
//  if( (res = recv(sock, buf, size, MSG_WAITALL)) == -1 ) {
//    throw TCPReadException(strerror(errno));
//  }

  // Wait until something is ready to be read ( peek'a'loop ).
  errno = EAGAIN;
  while( recv(sock, buf, 1, MSG_PEEK) == -1 && errno == EAGAIN) {
    usleep(100);
  }

  // Now read it
  if( (res = recv(sock, buf, size, MSG_DONTWAIT)) == -1 ) {
    throw TCPReadException(strerror(errno));
  }

  return res;
}
*/

int Pentominos::TCPSocket::write(char *data, int size)
{
  if(sock == -1) {
    throw TCPWriteException("Socket not initialized.");
  }

  if(!isconnected) {
    throw TCPWriteException("Socket is not connected.");
  }

  int res;
  if( (res = send(sock, data, size, MSG_WAITALL)) == -1 ) {
    throw TCPWriteException(strerror(errno));
  }

  return res;
}

std::string Pentominos::TCPSocket::srcaddr()
{
  std::string addr;

#ifndef BYPASS_STATICALLOCATIONS
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);
  if(getpeername(sock, (sockaddr*)&name, &namelen) == -1) {
    throw TCPNameException(strerror(errno));
  }

  addr = inet_ntoa(name.sin_addr);
#else/*BYPASS_STATICALLOCATIONS*/
  addr = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  return addr;
}

std::string Pentominos::TCPSocket::dstaddr()
{
  std::string addr;

#ifndef BYPASS_STATICALLOCATIONS
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);
  if(getsockname(sock, (sockaddr*)&name, &namelen) == -1) {
    throw TCPNameException(strerror(errno));
  }
  
  addr = inet_ntoa(name.sin_addr);
#else/*BYPASS_STATICALLOCATIONS*/
  addr = "127.0.0.1";
#endif/*BYPASS_STATICALLOCATIONS*/

  return addr;
}

#ifdef TEST_TCPSOCKET

int main()
{
  char buf[32];

  switch(fork()) {
  case -1: // error
    ERR(tcpsocket, "Could not fork: %s\n", strerror(errno));
    return 1;
        
  case 0: // child
    try {
      Pentominos::TCPSocket client;
      sleep(1); // We need to wait for the listen socket to be created.
      client.connect("localhost", 12345);
      sprintf(buf, "hello");
      client.write(buf, sizeof(buf));
      INFO(tcpsocket, "Sent: [%s]\n", buf);
    } catch( Pentominos::Exception &e ) {
      ERR(tcpsocket, "%s\n", e.what());
      return 1;
    }
    break;
        
  default: // parent
    try {
      Pentominos::TCPSocket listen_sock;
      listen_sock.listen(12345);
      Pentominos::TCPSocket sock = listen_sock.accept();
      sock.read(buf, sizeof(buf));
      INFO(tcpsocket, "Got: [%s]\n", buf);
      if(std::string(buf) != "hello") return 1;
    } catch( Pentominos::Exception &e ) {
      ERR(tcpsocket, "%s\n", e.what());
      return 1;
    }
    break;
  }

  return 0;
}

#endif/*TEST_TCPSOCKET*/
