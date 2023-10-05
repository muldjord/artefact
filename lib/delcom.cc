/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            delcom.cc
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
#include "delcom.h"

#include <hugin.hpp>

#define DELCOM_DEBUG

#define DELCOM_VENDOR 0x0FC5
#define DELCOM_PRODUCT 0x1222

Pentominos::Delcom::Delcom(USBInterface interface)
  : Pentominos::USB(DELCOM_VENDOR, DELCOM_PRODUCT, interface)
{
}

Pentominos::Delcom::~Delcom()
{
}

/*
	delcom_send_command
	-------------------
	Purpose: Send a command to the currently selected device
	Input:   A delcom_packet.  The only relative bits are:
           MinorCmd / MajorCmd
           DataLSB / DataMSB
           DataExtension
	Output:  0=Worked, -1=Failed
*/
int Pentominos::Delcom::sendCommand(struct delcom_packet *p)
{
  int len = p->Length;
  int requestType = 0xc8;
  
  if(p->MajorCmd == 11) {
    len = 8;
    requestType = 0xc8;
  } else {
    requestType = 0x48;
  }
        
  if ( len < 0 ) len = 0;
  if ( len > 8 ) len = 8; 

#ifdef DELCOM_DEBUG
  printf( "DELCOM> USB_CMD( %x 12 %x %x ptr %x )\n", 
          requestType,
          (p->MinorCmd * 0x100) + p->MajorCmd, 
          (p->DataLSB * 0x100) + p->DataMSB, 
          len );
#endif

	controlMessage(requestType,					// int requesttype
                 0x00000012,					// int request
                 (p->MinorCmd * 0x100) + p->MajorCmd,		// int value
                 (p->DataMSB * 0x100) + p->DataLSB,		// int index
                 (char *) p->DataExtension,			// char *bytes
                 len,	               				// int size
                 5000);						// int timeout

#ifdef DELCOM_DEBUG
  int i;
  printf("Length = %i\n", sizeof(struct delcom_packet));
  printf("  MajorCmd    0x%.2x\n", p->MajorCmd);
  printf("  MinorCmd    0x%.2x\n", p->MinorCmd);
  printf("  DataLSB     0x%.2x\n", p->DataLSB);
  printf("  DataMSB     0x%.2x\n", p->DataMSB);
  printf("  Length      0x%.4x\n", p->Length);
  for(i=0; i<8; i++) printf("%x ", p->DataExtension[i]);
  printf("\n");
#endif

	return 0;
}

#ifdef TEST_DELCOM

static int sendRawCmd(Pentominos::Delcom *delcom, int ma, int mi, int lsb, int msb, 
		      unsigned char *data, int datalen)
{
  struct Pentominos::delcom_packet p;

  memset(&p, 0, sizeof(p));
  p.MajorCmd = ma;
  p.MinorCmd = mi;
  p.DataLSB = lsb;
  p.DataMSB = msb;
  if( (datalen > 0) && (data != NULL) ) {
    if(datalen > 8)
      datalen = 8;
    //      return -1;
    p.Length = datalen;
    memcpy(p.DataExtension, data, datalen);
  }

  if(delcom->sendCommand(&p) < 0)
    return -1;

  return p.DataExtension[0] + (p.DataExtension[1] << 8);
}


int main()
{
  try {
    Pentominos::Delcom delcom;

    sendRawCmd(&delcom, 10, 3, 1, 0, NULL, 0);

  } catch(Pentominos::Exception &e) {
    fprintf(stderr, "Error: %s\n", e.what());
  }

  return 0;
}

#endif/*TEST_DELCOM*/
