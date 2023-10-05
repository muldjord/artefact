/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            md5.h
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
#ifndef __ARTEFACT_MD5_H__
#define __ARTEFACT_MD5_H__

#include <stdlib.h>
#include <openssl/md5.h>

#include <string>

#include <stdlib.h>

namespace Pentominos {

/**
 * This class is used to calculate the MD5 sum of a stream.
 */
class MD5Sum {
public:
  /**
   * Constructor.
   */
  MD5Sum();

  /**
   * Destructor.
   */
  ~MD5Sum();

  /**
   * Update the data (add data to the sum)
   * @param buf The char buffer containing the data to be added.
   * @param size The size of the data contained in buf.
   */
  void update(char *buf, size_t size);

  /**
   * Update the data (add data to the sum)
   * @param str The string containing the data to be added.
   */
  void update(std::string str);
  
  /**
   * Stop the calculation of the sum (no more data can be added after this point),
   * and return it.
   */
  std::string getSum();

private:
  MD5_CTX c;
};

};

#endif/*__ARTEFACT_MD5_H__*/
