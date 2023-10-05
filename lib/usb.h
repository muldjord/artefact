/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            usb.h
 *
 *  Thu Jun 21 16:06:49 CEST 2007
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
#ifndef __PENTOMINOS_USB_H__
#define __PENTOMINOS_USB_H__

#include <string>

#include "iodevice.h"

#include "exception.h"

namespace Pentominos {

  /**
   * USB Exceptions
   */
  class USBException : public Pentominos::Exception {
  public:
    USBException(std::string reason)
      : Pentominos::Exception("Error in usb: " +  reason) {}
  };

  /**
   * This exception is thrown if the requested device cannot be found.
   */
  class USBDeviceException : public Pentominos::Exception {
  public:
    USBDeviceException(std::string device)
      : Pentominos::Exception("USB could not find device: " +  device ) {}
  };

  typedef int USBInterface;
  typedef int USBVendor;
  typedef int USBProduct;

  class USB : public IODevice {
  public:
    USB(USBVendor vendor, USBProduct product, USBInterface interface = 0) throw(USBDeviceException);
    ~USB();

    char *read(int timeout, int *size);
    void write(char *data, size_t size);
    
    void controlMessage(int requesttype,
                        int request,
                        int value,
                        int index,
                        char *bytes,
                        int size,
                        int timeout);


  protected:
    void *handle;
    USBInterface interface;
    USBVendor vendor;
    USBProduct product;
  };

};

#endif/*__PENTOMINOS_USB_H__*/
