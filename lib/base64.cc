/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            base64.cc
 *
 *  Mon Feb  8 10:25:52 CET 2010
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
#include "base64.h"

#include <memory.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>

Pentominos::Base64Encoder::Base64Encoder()
{
  mbio = (void*)BIO_new(BIO_s_mem());
  BIO *b64bio = BIO_new(BIO_f_base64());
  bio = (void*)BIO_push(b64bio, (BIO*)mbio);
}

Pentominos::Base64Encoder::~Base64Encoder()
{
  BIO_free_all((BIO*)bio);
}

std::string Pentominos::Base64Encoder::write(std::string in)
{
  return this->write(in.data(), in.length());
}

std::string Pentominos::Base64Encoder::write(const char *in, size_t size)
{
  std::string out;

  BIO_write((BIO*)bio, in, size);

  size_t osize = BIO_ctrl_pending((BIO*)mbio);
  char *outbuf = (char*)malloc(osize);

  int len = BIO_read((BIO*)mbio, outbuf, osize);
  if(len < 1) return "";
  out.append(outbuf, len);

  free(outbuf);

  return out;
}

std::string Pentominos::Base64Encoder::flush()
{
  std::string out;

  (void)BIO_flush((BIO*)bio);

  size_t size = BIO_ctrl_pending((BIO*)mbio);
  char *outbuf = (char*)malloc(size);

  int len = BIO_read((BIO*)mbio, outbuf, size);
  if(len < 1) return "";
  out.append(outbuf, len);

  free(outbuf);

  return out;
}

Pentominos::Base64Decoder::Base64Decoder()
{
  mbio = (void*)BIO_new(BIO_s_mem());
  BIO_set_mem_eof_return((BIO*)mbio, -1);

  BIO *b64bio = BIO_new(BIO_f_base64());
  BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);

  bio = (void*)BIO_push(b64bio, (BIO*)mbio);
  //  BIO_set_callback((BIO*)mbio,BIO_debug_callback);
}

Pentominos::Base64Decoder::~Base64Decoder()
{
  BIO_free_all((BIO*)bio);
}

std::string Pentominos::Base64Decoder::write(std::string in)
{
  return this->write(in.data(), in.length());
}

std::string Pentominos::Base64Decoder::write(const char *_in, size_t size)
{
  std::string feed;

  const unsigned char *in = (const unsigned char *)_in;

  // Test if the string contains non-valid base64 characters.
  for(size_t i = 0; i < size; i++) {
    if(in[i] == ' ' || in[i] == '\n' || in[i] == '\t' ||
       in[i] == '\r' || in[i] == 0xa0) continue;
    if(in[i] == '+' || in[i] == '/'  || in[i] == '=') {
      feed += in[i];
      continue;
    }
    if((in[i] >=  '0' && in[i] <=  '9') ||
       (in[i] >=  'a' && in[i] <=  'z')  ||
       (in[i] >=  'A' && in[i] <=  'Z')) {
      feed += in[i];
      continue;
    }
    throw MalformedInputException();
  }

  std::string out;

  BIO_write((BIO*)mbio, feed.data(), feed.length());
  
  int len;
  while(1) {
    char buf[32];
    len = BIO_read((BIO*)bio, buf, sizeof(buf));
    if(len <= 0) break;
    out.append(buf, len);
  }
  
  return out;
}

std::string Pentominos::base64encode(char *in, unsigned int size)
{
  Pentominos::Base64Encoder enc;
  std::string out = enc.write(in, size);
  out += enc.flush();
  return out;
}

std::string Pentominos::base64encode(std::string in)
{
  Pentominos::Base64Encoder enc;
  std::string out = enc.write(in);
  out += enc.flush();
  return out;
}

std::string Pentominos::base64decode(char *in, unsigned int size)
{
  Pentominos::Base64Decoder dec;
  return dec.write(in, size);
}

std::string Pentominos::base64decode(std::string in)
{
  Pentominos::Base64Decoder dec;
  return dec.write(in);
}

#ifdef TEST_BASE64
//deps:
//cflags: $(OPENSSL_CFLAGS) -I..
//libs: $(OPENSSL_LIBS)
#include "test.h"

#define REF "PERlbm5lIHN0cmVuZyBtw6UgdsOmcmUgbGlkdCBpbnRlcnJlc3NhbnQuPg==\n"

#define REFLONG \
 "PERlbm5lIHN0cmVuZyBtw6UgdsOmcmUgbGlkdCBpbnRlcnJlc3NhbnQuPjxEZW5u\n" \
 "ZSBzdHJlbmcgbcOlIHbDpnJlIGxpZHQgaW50ZXJyZXNzYW50Lj48RGVubmUgc3Ry\n" \
 "ZW5nIG3DpSB2w6ZyZSBsaWR0IGludGVycmVzc2FudC4+PERlbm5lIHN0cmVuZyBt\n" \
 "w6UgdsOmcmUgbGlkdCBpbnRlcnJlc3NhbnQuPjxEZW5uZSBzdHJlbmcgbcOlIHbD\n" \
 "pnJlIGxpZHQgaW50ZXJyZXNzYW50Lj4=\n"

#define LONG                                  \
  "<Denne streng må være lidt interressant.>" \
  "<Denne streng må være lidt interressant.>" \
  "<Denne streng må være lidt interressant.>" \
  "<Denne streng må være lidt interressant.>" \
  "<Denne streng må være lidt interressant.>"

TEST_BEGIN;

std::string ref = "<Denne streng må være lidt interressant.>";

{ // Isolated decoding
  std::string val = Pentominos::base64decode(REF);
  TEST_EQUAL_STR(ref, val, "Are they still the same?");
}

{ // Short input
  std::string b64 = Pentominos::base64encode(ref);
  TEST_GREATER_THAN_INT(b64.length(), ref.length(), "Test if the string got longer.");
    
  TEST_EQUAL_STR(std::string(REF), b64, "Compare to reference value.");
  
  std::string val = Pentominos::base64decode(b64);
  TEST_EQUAL_STR(ref, val, "Are they still the same?");
}

{ // Long input
  std::string reflong = LONG;

  std::string b64long = Pentominos::base64encode(reflong);
  TEST_GREATER_THAN_INT(b64long.length(), reflong.length(),
                        "Test if the string got longer.");

  TEST_EQUAL_STR(std::string(REFLONG), b64long, "Compare to reference value.");
  
  std::string vallong = Pentominos::base64decode(b64long);
  TEST_EQUAL_STR(reflong, vallong, "Are they still the same?");
}

{ // Long input fed in small chunks
  std::string reflong = REFLONG;

  std::string dec;
  size_t p = 0;
  Pentominos::Base64Decoder decoder;
  for(size_t sz = 0; sz < 100; sz++) {
    if(p < reflong.length()) {
      if(p + sz > reflong.length()) sz = reflong.length() - p;
      std::string d = decoder.write(reflong.data() + p, sz);
      dec.append(d.data(), d.length());
      p += sz;
    } else {
      break;
    }
  }

  TEST_EQUAL_STR(std::string(LONG), dec, "Compare to reference value.");
}

{ // Long decoding input fed in small chunks
  std::string reflong = LONG;

  std::string enc;
  size_t p = 0;
  Pentominos::Base64Encoder encoder;
  for(size_t sz = 0; sz < 100; sz++) {
    if(p < reflong.length()) {
      if(p + sz > reflong.length()) sz = reflong.length() - p;
      std::string d = encoder.write(reflong.data() + p, sz);
      enc.append(d.data(), d.length());
      p += sz;
    } else {
      break;
    }
  }
  std::string d = encoder.flush();
  enc.append(d.data(), d.length());

  TEST_EQUAL_STR(std::string(REFLONG), enc, "Compare to reference value.");
}


{ // Emtpy input
  Pentominos::Base64Encoder enc;
  std::string b64 = enc.write("");
  b64 += enc.flush();
  TEST_EQUAL_STR(std::string(""), "", "Compare to reference value.");
}

{ // Class test
  Pentominos::Base64Encoder enc;
  std::string b64 = enc.write(ref);
  b64 += enc.flush();
  TEST_EQUAL_STR(std::string(REF), b64, "Compare to reference value.");
}

{ // Empty input
  Pentominos::Base64Decoder dec;
  std::string val = dec.write("");
  TEST_EQUAL_STR("", val, "Compare to reference value.");
}

{ // Class test
  Pentominos::Base64Decoder dec;
  std::string val = dec.write(REF);
  TEST_EQUAL_STR(ref, val, "Compare to reference value.");
}

{ // Invalid input test
  std::string bad_b64 = "\240\250\242\243";
  Pentominos::Base64Decoder dec;
  TEST_EXCEPTION(dec.write(bad_b64),
                 Pentominos::Base64Decoder::MalformedInputException,
                 "Malformed input exception."); 
}

TEST_END;

#endif/*TEST_BASE64*/
