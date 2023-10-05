/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            delcom.h
 *
 *  Mon Jul  2 16:04:10 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
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
#ifndef __PENTOMINOS_DELCOM_H__
#define __PENTOMINOS_DELCOM_H__

#include "usb.h"

namespace Pentominos {

  /*
    This struct has a lot of useless data in it
    I took this from the windows driver released
    by Delcom
  */
  struct delcom_packet {
    unsigned char MajorCmd;
    unsigned char MinorCmd;
    unsigned char DataLSB;
    unsigned char DataMSB;
    unsigned short Length;
    unsigned char DataExtension[8];
  };

  class Delcom : public USB {
  public:
    Delcom(USBInterface interface = 0);
    ~Delcom();
    
    int sendCommand(struct delcom_packet *p);
  };

};

#endif/*__PENTOMINOS_DELCOM_H__*/
