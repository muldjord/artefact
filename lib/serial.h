/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <string>
#include <termios.h>

#include "iodevice.h"

#include "exception.h"

namespace Pentominos {

  /**
   * Serial Exceptions
   */
  class SerialException : public Pentominos::Exception {
  public:
    SerialException(std::string reason)
      : Pentominos::Exception("Error in serial: " +  reason) {}
  };
  
  /**
   * The exception in thrown by Serial when a read error occurs.
   */
  class SerialReadException : public Pentominos::Exception {
  public:
    SerialReadException(std::string reason)
      : Pentominos::Exception("Error while reading in serial: " +  reason) {}
  };


  typedef enum {
    PARITY_NONE,
    PARITY_EVEN,
    PARITY_ODD
  } parity_t;
  
  typedef enum {
    BITS_1,
    BITS_1_5,
    BITS_2
  } bits_t;
  
  class Serial : public IODevice {
  public:
    Serial(std::string device = "/dev/ttyS0",
           speed_t speed = B9600,
           unsigned char wordlength = 8,
           bits_t startbits = BITS_2,
           bits_t stopbits = BITS_2,
           parity_t parity = PARITY_NONE);
    ~Serial();
    
    void changeDevice(std::string device);
    void changeSpeed(speed_t speed);
    void changeStartbits(bits_t startbits);
    void changeStopbits(bits_t stopbits);
    void changeWordLength(unsigned int databits);
    void changeParity(parity_t parity);
    
    /**
     * Reinitialize the serial connection (after changing some of the parameters).
     */
    void reinit();
    
    /**
     * This method read some bytes from the serial port.
     * @param timeout The timeout in miliseconds. A timeout will return the read
     * bytes in the buffer (possibly empty).
     * @param size A pointer to an integer, which will contain the number of bytes
     * read when the call is finished, og posibly -1 if an error occurred.
     * @return A char pointer to an internal buffer of the object (i.e. the caller
     * should NOT free it.)\n
     * The buffer is zero terminated.
     */
    char *read(int timeout, int *size) throw(SerialReadException);
    //  int read(char *buf, int size);
    void write(char *data, size_t size);
    
  private:
    int fd;
    char buf[1024];
    
    std::string device;
    speed_t speed;
    bits_t stopbits;
    bits_t startbits;
    unsigned char wordlength;
    parity_t parity;
  };

};

#endif/*__SERIAL_H__*/
