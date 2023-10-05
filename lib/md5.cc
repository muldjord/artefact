/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            md5.cc
 *
 *  Mon Apr 16 13:22:23 CEST 2007
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
#include "md5.h"

#include <hugin.hpp>
#include <string.h>

/*
unsigned char *MD5(const unsigned char *d, unsigned long n,
                   unsigned char *md);

int MD5_Init(MD5_CTX *c);
int MD5_Update(MD5_CTX *c, const void *data,
               unsigned long len);
int MD5_Final(unsigned char *md, MD5_CTX *c);
*/

Pentominos::MD5Sum::MD5Sum()
{
  MD5_Init(&c);
}

Pentominos::MD5Sum::~MD5Sum()
{
}

void Pentominos::MD5Sum::update(char *buf, size_t size)
{
  MD5_Update(&c, buf, size);
}

void Pentominos::MD5Sum::update(std::string str)
{
  MD5_Update(&c, str.c_str(), str.length());
}

std::string Pentominos::MD5Sum::getSum()
{
  std::string sum;
  unsigned char md5[MD5_DIGEST_LENGTH + 1];
  memset(md5, 0, sizeof(md5));

  MD5_Final(md5, &c);

  for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    char buf[3];
    sprintf(buf, "%02x", md5[i]);
    sum.append(buf);
  }

  return sum;
}

#ifdef TEST_MD5
/**
 * Compile:
 * g++ -DTEST_MD5 md5.cc -lcrypto -o md5_test
 */
int main(int argc, char *argv[])
{
  Pentominos::MD5Sum md5;

  md5.update("dingding", 8);

  printf("MD5 %s\n", md5.getSum().c_str());

  return 0;
}

#endif/*TEST_MD5*/
