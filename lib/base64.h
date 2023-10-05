/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            base64.h
 *
 *  Fri Oct 20 15:25:39 CEST 2006
 *  Copyright  2006 Peter Skaarup
 *  piparum@piparum.dk
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
#ifndef __ARTEFACT_BASE64_H__
#define __ARTEFACT_BASE64_H__

#include <string>
#include <exception>

namespace Pentominos {

  class Base64Encoder {
  public:
    Base64Encoder();
    ~Base64Encoder();

    std::string write(std::string in);
    std::string write(const char *buf, size_t size);
    std::string flush();

  private:
    void *bio;
    void *mbio;
  };

  class Base64Decoder {
  public:
    class MalformedInputException : public std::exception {};

    Base64Decoder();
    ~Base64Decoder();

    std::string write(std::string in);
    std::string write(const char *buf, size_t size);

  private:
    void *bio;
    void *mbio;
  };

  std::string base64encode(char *in, unsigned int size);
  std::string base64encode(std::string in);

  std::string base64decode(char *in, unsigned int size);
  std::string base64decode(std::string in);

};

#endif/*__ARTEFACT_BASE64_H__*/
