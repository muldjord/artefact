/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tostring.h
 *
 *  Thu Mar 29 13:16:13 CEST 2007
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
#ifndef __ARTEFACT_TOSTRING_H__
#define __ARTEFACT_TOSTRING_H__

#include <string>

namespace Pentominos {
  /**
   * toString converts a nonstring variable into an STL string.
   * @param s A string, converted into a... string...
   * @return The STL string containing the converted value.
   */
  std::string toString(std::string s);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param c A char, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(char c);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param c A unsigned char, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(unsigned char c);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param si A short integer, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(short int si);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param su An unsigned short integer, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(short unsigned int su);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param li A long integer, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(int li);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param lu An unsigned long integer, converted into a string.
   * @return The STL string containing the converted value.
   */
  std::string toString(unsigned int lu);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param b A boolean value, converted into a string (true/false).
   * @return The STL string containing the converted value.
   */
  std::string toString(bool b);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param f A floating point value, converted into a string.
   * @param precision The precision to use when converting.
   * @return The STL string containing the converted value.
   */
  std::string toString(float f, unsigned int precision = 8);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param d A double precision floating point value, converted into a string.
   * @param precision The precision to use when converting.
   * @return The STL string containing the converted value.
   */
  std::string toString(double d, unsigned int precision = 16);

  /**
   * toString converts a nonstring variable into an STL string.
   * @param ld A long double precision floating point value, converted into a string.
   * @param precision The precision to use when converting.
   * @return The STL string containing the converted value.
   */
  std::string toString(long double ld, unsigned int precision = 32);
};

#endif/*__ARTEFACT_TOSTRING_H__*/
