/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            usb.cc
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
#include "usb.h"
#include LIBUSB_HEADER_FILE

#include <hugin.hpp>

#include "tostring.h"

Pentominos::USB::USB(USBVendor vendor, USBProduct product, USBInterface interface)
  throw(USBDeviceException)
{
  //  struct usb_bus *busses;
  //  struct usb_device *dev;

  handle = NULL;
  this->interface = interface;
  this->vendor = vendor;
  this->product = product;
  /*
  usb_init();
  usb_find_busses();
  usb_find_devices();
    
  busses = usb_get_busses();

  struct usb_bus *bus;
  int c, i, a;
    
  for (bus = busses; bus; bus = bus->next) {
    struct usb_device *dev;
    
    for (dev = bus->devices; dev; dev = dev->next) {
      // Check if this device is the correct type
      if (dev->descriptor.bDeviceClass == this->interface) {
        // Open the device, claim the interface and do your processing
        handle = usb_open(dev);
        usb_claim_interface((usb_dev_handle *)handle, this->interface);
      }
    
      // Loop through all of the configurations
      for (c = 0; c < dev->descriptor.bNumConfigurations; c++) {
        // Loop through all of the interfaces
        for (i = 0; i < dev->config[c].bNumInterfaces; i++) {
          // Loop through all of the alternate settings
          for (a = 0; a < dev->config[c].interface[i].num_altsetting; a++) {
            // Check if this interface is the correct type
            if (dev->config[c].interface[i].altsetting[a].bInterfaceClass == this->interface) {
              // Open the device, set the alternate setting, claim the interface and do your processing
              handle = usb_open(dev);
              usb_claim_interface((usb_dev_handle *)handle, this->interface);
            }
          }
        }
      }
    }
  }
  */

	usb_init();
	usb_find_busses();
	usb_find_devices();

  for(struct usb_bus *bus = usb_busses; bus;  bus = bus->next) {
		for(struct usb_device *dev = bus->devices; dev; dev = dev->next) {
			if((dev->descriptor.idVendor == vendor) && (dev->descriptor.idProduct == product)) {
				// Open the device
        handle = usb_open(dev);
        if(handle == NULL)
          throw USBDeviceException(usb_strerror());

        int if_num = 0;
        int ret;

        int i,j,k;
        for (i = 0; i < dev->descriptor.bNumConfigurations; i++) {
          struct usb_config_descriptor *config = &dev->config[i];
          for (j = 0; j < config->bNumInterfaces; j++) {
            struct usb_interface *interface = &config->interface[j];
            for (k = 0; k < interface->num_altsetting; k++) {
              struct usb_interface_descriptor *altinterface = &interface->altsetting[k];
              if_num = altinterface->bInterfaceNumber;
            }
          }
        }

        // Set the configuration
        if( usb_set_configuration((usb_dev_handle*)handle, 1) < 0 ) {
          throw USBDeviceException(usb_strerror());
          //printf( "DELCOM> USB Error (usb_set_configuration): %s\n", usb_strerror() );
          //          return -1;
        }

        // Claim the interface
        ret = usb_claim_interface((usb_dev_handle*)handle, if_num);
        if( ret < 0 ) {
          throw USBDeviceException(usb_strerror());
          //          printf( "DELCOM> USB Error (usb_claim_interface): %s\n", usb_strerror() );
          //          return -1;
        }

        // Set the Alt Interface
        ret = usb_set_altinterface((usb_dev_handle*)handle, 0);
        if( ret < 0 ) {
          throw USBDeviceException(usb_strerror());
          //          printf( "DELCOM> USB Error (usb_set_altinterface): %s\n", usb_strerror() );
          //          return -1;
        }
			}
		}
	}

  if(handle == NULL)
    throw USBDeviceException(Pentominos::toString(this->interface));
}

Pentominos::USB::~USB()
{
  usb_dev_handle *usb = (usb_dev_handle *)handle;
  usb_release_interface(usb, this->interface);
  usb_close(usb);
}

char *Pentominos::USB::read(int timeout, int *size)
{
  usb_dev_handle *usb = (usb_dev_handle *)handle;

  usb = usb;

  *size = -1;
  return NULL;
}

void Pentominos::USB::write(char *data, size_t size)
{
  usb_dev_handle *usb = (usb_dev_handle *)handle;
  usb = usb;
}

void Pentominos::USB::controlMessage(int requesttype,
                                     int request,
                                     int value,
                                     int index,
                                     char *bytes,
                                     int size,
                                     int timeout)
{
  usb_dev_handle *usb = (usb_dev_handle *)handle;
  int ret = usb_control_msg(usb, requesttype, request, value, index, bytes, size, timeout);
  if(ret < 0) throw USBException("usb_control_msg");
}

#ifdef TEST_USB

int main()
{
  try {
    Pentominos::USB usb(0,0,99);
    return 1;
  } catch( Pentominos::USBDeviceException &e ) {
    fprintf(stderr, "%s\n", e.what());
  }

  return 0;
}

#endif/*TEST_USB*/
