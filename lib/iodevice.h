/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            iodevice.h
 *
 *  Wed May 16 14:09:57 CEST 2007
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
#ifndef __ARTEFACT_IODEVICE_H__
#define __ARTEFACT_IODEVICE_H__

#include <stdlib.h>

class IODevice {
public:
  virtual ~IODevice() {}

  /**
   * Read chunk of bytes from input.
   * @param timeout The read timeout in milliseconds
   * @param size An int pointer which will contain the number of bytes read or
   * -1 on timeout.
   */
  virtual char *read(int timeout, int *size) = 0;

  /**
   * Write a char buffer to the output.
   * @param data A pointer to the char buffer containing the data to write.
   * @param size The size of the data buffer to write.
   */
  virtual void write(char *data, size_t size) = 0;
};


#endif/*__ARTEFACT_IODEVICE_H__*/
