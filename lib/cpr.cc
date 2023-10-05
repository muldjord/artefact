/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            cpr.cc
 *
 *  Wed Feb 21 12:01:21 CET 2007
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
#include "cpr.h"

#include <hugin.hpp>

// For gethostbyname
#include <netdb.h>

// For inet_addr
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For connect, listen, bind and accept
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
#include <stdlib.h>

Pentominos::CPR::CPR(std::string cpr, std::string server, unsigned short int port)
{
  int sock;
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  
  _valid = false;
 
  // Prepare socket.
  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    throw CPRConnectionException(strerror(errno));
  }

#ifndef BYPASS_STATICALLOCATIONS

	struct hostent *he = gethostbyname(server.c_str());
	if(!he) {
    throw CPRConnectionException(strerror(errno));
	}

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
	socketaddr.sin_addr.s_addr = *((unsigned long *)he->h_addr);

#else/*BYPASS_STATICALLOCATIONS*/

  struct sockaddr_in socketaddr;
  memset((char *) &socketaddr, 0, sizeof(socketaddr));
  socketaddr.sin_family = AF_INET;
  socketaddr.sin_port = htons(port);
  socketaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

#endif/*BYPASS_STATICALLOCATIONS*/
  
  if(connect(sock, (struct sockaddr*)&socketaddr, sizeof(socketaddr))) {
    throw CPRConnectionException(strerror(errno));
  }
  
  // Send CPR number
  cpr.append("\n");
  if(send(sock, cpr.c_str(), cpr.length(), MSG_WAITALL) != (int)cpr.length()) {
    throw CPRConnectionException(strerror(errno));
  }

  // Read result
  if(recv(sock, buf, sizeof(buf), MSG_WAITALL) < 1) {
    //    throw CPRConnectionException(strerror(errno));
    throw CPRMalformedException(); // CPR server terminates connection on malformed input.
  }

  close(sock);

  // Parse result
  std::string result = buf;

  while(result.length() > 0) {
    std::string::size_type newline = result.find("\n");
    std::string line = result.substr(0, newline);

    std::string code = line.substr(0, 4);
    if(code == "0000") _cpr = line.substr(4, std::string::npos);
    if(code == "0001") _efternavn = line.substr(4, std::string::npos);
    if(code == "0002") _fornavn = line.substr(4, std::string::npos);
    if(code == "0003") _conavn = line.substr(4, std::string::npos);
    if(code == "0004") _lokalitet = line.substr(4, std::string::npos);
    if(code == "0005") _adresse = line.substr(4, std::string::npos);
    if(code == "0006") _bynavn = line.substr(4, std::string::npos);
    if(code == "0007") _postnr = line.substr(4, std::string::npos);
    if(code == "0008") _postby = line.substr(4, std::string::npos);
    if(code == "0009") _kommunenr = line.substr(4, std::string::npos);
    if(code == "000a") _sikringsgruppe = line.substr(4, std::string::npos);
    if(code == "000b") _laegenr = line.substr(4, std::string::npos);
    if(code == "000c") _beskyttelse = line.substr(4, std::string::npos);
    if(code == "000d") _reserveret = line.substr(4, std::string::npos);
    if(code == "000e") _aktion = line.substr(4, std::string::npos);
    if(code == "000f") _doedsdato = line.substr(4, std::string::npos);
    if(code == "0010") _insert_at = line.substr(4, std::string::npos);
    if(code == "0011") _moved_at = line.substr(4, std::string::npos);
    if(code == "00ff") _num_fields = line.substr(4, std::string::npos);

    result = result.substr(newline+1, std::string::npos);
  }

  if(_cpr == "") {
    _cpr = cpr;
    throw CPRNotFoundException();
  }

  _valid = true;
}

bool Pentominos::CPR::valid()
{
  return _valid;
}

std::string Pentominos::CPR::cpr()
{
  return _cpr;
}

std::string Pentominos::CPR::efternavn()
{
  return _efternavn;
}

std::string Pentominos::CPR::fornavn()
{
  return _fornavn;
}

std::string Pentominos::CPR::conavn()
{
  return _conavn;
}

std::string Pentominos::CPR::lokalitet()
{
  return _lokalitet;
}

std::string Pentominos::CPR::adresse()
{
  return _adresse;
}

std::string Pentominos::CPR::bynavn()
{
  return _bynavn;
}

std::string Pentominos::CPR::postnr()
{
  return _postnr;
}

std::string Pentominos::CPR::postby()
{
  return _postby;
}

std::string Pentominos::CPR::kommunenr()
{
  return _kommunenr;
}

std::string Pentominos::CPR::sikringsgruppe()
{
  return _sikringsgruppe;
}

std::string Pentominos::CPR::laegenr()
{
  return _laegenr;
}

std::string Pentominos::CPR::beskyttelse()
{
  return _beskyttelse;
}

std::string Pentominos::CPR::reserveret()
{
  return _reserveret;
}

std::string Pentominos::CPR::aktion()
{
  return _aktion;
}

std::string Pentominos::CPR::doedsdato()
{
  return _doedsdato;
}

std::string Pentominos::CPR::insert_at()
{
  return _insert_at;
}

std::string Pentominos::CPR::moved_at()
{
  return _moved_at;
}

std::string Pentominos::CPR::num_fields()
{
  return _num_fields;
}

int Pentominos::CPR::birthday()
{
  return atoi(_cpr.substr(0,2).c_str());
}

int Pentominos::CPR::birthmonth()
{
  return atoi(_cpr.substr(2,2).c_str());
}

int Pentominos::CPR::birthyear()
{
  int year = atoi(_cpr.substr(4,2).c_str());
  int control = atoi(_cpr.substr(6,4).c_str());

  if(year >=  0 && year <= 99 && control >= 1    && control <=  999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 1000 && control <= 1999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 2000 && control <= 2999) return year + 1900;
  if(year >=  0 && year <= 99 && control >= 3000 && control <= 3999) return year + 1900;
  if(year >=  0 && year <= 36 && control >= 4000 && control <= 4999) return year + 2000;
  if(year >= 37 && year <= 99 && control >= 4000 && control <= 4999) return year + 1900;
  if(year >=  0 && year <= 36 && control >= 5000 && control <= 5999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 5000 && control <= 5999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 6000 && control <= 6999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 6000 && control <= 6999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 7000 && control <= 7999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 7000 && control <= 7999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 8000 && control <= 8999) return year + 2000;
  // Unused: 37 <= year <= 57
  if(year >= 58 && year <= 99 && control >= 8000 && control <= 8999) return year + 1800;
  if(year >=  0 && year <= 36 && control >= 9000 && control <= 9999) return year + 2000;
  if(year >= 37 && year <= 99 && control >= 9000 && control <= 9999) return year + 1900;

  return year;
}

Pentominos::sex_t Pentominos::CPR::sex()
{
  int control = atoi(_cpr.substr(6,4).c_str());
  if(control % 2 == 0) return sex_female;
  return sex_male;
}

#ifdef TEST_CPR
int main(int argc, char *argv[])
{
  try {
    Pentominos::CPR cpr("2003791613");

    if(cpr.valid()) {
      printf("%s\n", cpr.cpr().c_str());
      printf("%s\n", cpr.efternavn().c_str());
      printf("%s\n", cpr.fornavn().c_str());
      printf("%s\n", cpr.conavn().c_str());
      printf("%s\n", cpr.lokalitet().c_str());
      printf("%s\n", cpr.adresse().c_str());
      printf("%s\n", cpr.bynavn().c_str());
      printf("%s\n", cpr.postnr().c_str());
      printf("%s\n", cpr.postby().c_str());
      printf("%s\n", cpr.kommunenr().c_str());
      printf("%s\n", cpr.sikringsgruppe().c_str());
      printf("%s\n", cpr.laegenr().c_str());
      printf("%s\n", cpr.beskyttelse().c_str());
      printf("%s\n", cpr.reserveret().c_str());
      printf("%s\n", cpr.aktion().c_str());
      printf("%s\n", cpr.doedsdato().c_str());
      printf("%s\n", cpr.insert_at().c_str());
      printf("%s\n", cpr.moved_at().c_str());
      printf("%s\n", cpr.num_fields().c_str());
      printf("=====\n");
      printf("Birthday: %d/%d - %d\n", cpr.birthday(), cpr.birthmonth(), cpr.birthyear());
      printf("Sex: %s (%d)\n", cpr.sex()==Pentominos::sex_male?"male":"female", cpr.sex());
    }
  } catch( Pentominos::Exception &e ) {
    fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }

  try {
    Pentominos::CPR cpr("0000000000"); // Wellformed but unknown cpr.
    return 1; // This should not succeed!
  } catch( Pentominos::CPRNotFoundException &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
  } catch( Pentominos::Exception &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
    return 1;
  }

  try {
    Pentominos::CPR cpr("1234567890"); // Malformed cpr (doesn't pass the mod 11 test)
    return 1; // This should not succeed!
  } catch( Pentominos::CPRMalformedException &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
  } catch( Pentominos::Exception &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
    return 1;
  }

  try {
    Pentominos::CPR cpr("2003791613", "localhost", 12345); // Wrong server.
    return 1; // This should not succeed!
  } catch( Pentominos::CPRConnectionException &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
  } catch( Pentominos::Exception &e ) {
    fprintf(stdout, "Error: %s\n", e.what());
    return 1;
  }

  return 0;
}

#endif//TEST_CPR
