/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include "serial.h"
using namespace std;

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

//#include <signal.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h> 
//#include <netinet/in.h>
//#include <math.h>
//#include <time.h>


//#define SPEED B0
//#define SPEED B50
//#define SPEED B75
//#define SPEED B110
//#define SPEED B134
//#define SPEED B150
//#define SPEED B200
//#define SPEED B300
//#define SPEED B600
//#define SPEED B1200
//#define SPEED B1800
//#define SPEED B2400
//#define SPEED B4800
//#define SPEED B9600
//#define SPEED B19200
//#define SPEED B38400
//#define SPEED B57600
//#define SPEED B115200
//#define SPEED B230400

#define BLOCK_SIZE 1 // in bytes
#define TIMEOUT 1 // in tics (tenth of seconds)

Pentominos::Serial::Serial(string dev,
               speed_t spd,
               unsigned char wlen,
               bits_t start,
               bits_t stop,
               parity_t par)
{
  device = dev;
  speed = spd;
  wordlength = wlen;
  startbits = start;
  stopbits = stop;
  parity = par;

  fd = -1;
  reinit();
}

Pentominos::Serial::~Serial()
{
	if(fd > 0) close(fd);
}

void Pentominos::Serial::reinit()
{
  int status;

  // Open the device
  if(fd > 0) close(fd);
  //  printf("opening [%s]...\n", device.c_str());
  fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(fd < 0) {
    throw SerialException(std::string("Device '") + device +
                          "' not opened: " + strerror(errno));
  }

  // Set options
  struct termios options;
  tcsetattr(fd, TCSANOW, &options);
  
  tcgetattr(fd, &options);
  cfmakeraw(&options);

  unsigned int input_parity_flags;
  switch(parity) {
  case PARITY_NONE:
    input_parity_flags = IGNPAR;
    break;
  case PARITY_EVEN:
    input_parity_flags = INPCK;
    break;
  case PARITY_ODD:
    input_parity_flags = INPCK;
    break;
  default:
    throw SerialException("Invalid parity check mode");
    break;
  }
  // tcflag_t c_iflag;      /* input modes */
  options.c_iflag = IGNBRK | input_parity_flags;

  // tcflag_t c_oflag;      /* output modes */
  options.c_oflag = NL0 | CR0 | TAB0 | BS0 | VT0 | FF0;

  // tcflag_t c_cflag;      /* control modes */
  unsigned int character_size_mask;
  switch(wordlength) {
  case 5:
    character_size_mask = CS5;
    break;
  case 6:
    character_size_mask = CS6;
    break;
  case 7:
    character_size_mask = CS7;
    break;
  case 8:
    character_size_mask = CS8;
    break;
  default:
    throw SerialException("Invalid wordlength");
    break;
  }

  unsigned int parity_flags;
  switch(parity) {
  case PARITY_NONE:
    parity_flags = 0;
    break;
  case PARITY_EVEN:
    parity_flags = PARENB;
    break;
  case PARITY_ODD:
    parity_flags = PARENB | PARODD;
    break;
  default:
    throw SerialException("Invalid parity check mode");
    break;
  }

  unsigned int stopbits_flags;
  switch(stopbits) {
  case BITS_1:
    stopbits_flags = 0;
    break;
  case BITS_1_5:
    stopbits_flags = 0;
    throw SerialException("1.5 Stopbits is unimplemented"); 
    break;
  case BITS_2:
    stopbits_flags = CSTOPB;
    break;
  default:
    throw SerialException("Invalid number of stopbits");
    break;
  }

  options.c_cflag = character_size_mask | CREAD | CLOCAL | parity_flags | stopbits_flags;

  // tcflag_t c_lflag;      /* local modes */
  options.c_lflag = 0;

  // cc_t c_cc[NCCS];       /* control chars */
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = TIMEOUT;


  // Set speed
	cfsetospeed(&options, speed);
  cfsetispeed(&options, speed);

  if(tcsetattr(fd, TCSANOW, &options) < 0)
    throw SerialException(strerror(errno));

  ioctl(fd, TIOCMGET, &status);
  status &= ~TIOCM_RTS;
  status |= TIOCM_DTR;
  ioctl(fd, TIOCMSET, &status);
  
	//  sleep (5);
  
  ioctl(fd, TIOCMGET, &status);
  status |= TIOCM_RTS;
  status &= ~TIOCM_DTR;
  ioctl(fd, TIOCMSET, &status);
  
	//  sleep (5);
}

void Pentominos::Serial::changeParity(parity_t p)
{
  parity = p;
}

void Pentominos::Serial::changeStartbits(bits_t s)
{
  startbits = s;
}

void Pentominos::Serial::changeStopbits(bits_t s)
{
  stopbits = s;
}

void Pentominos::Serial::changeWordLength(unsigned int w)
{
  wordlength = w;
}

void Pentominos::Serial::changeSpeed(speed_t s)
{
  speed = s;
}

void Pentominos::Serial::changeDevice(string d)
{
  device = d;
}

static ssize_t _read(int fd, void *buf, size_t count) { return read(fd, buf, count); }
/*
int Serial::read(char *buf, int size)
{
	if(fd < 0) throw SerialException("Uninitialized!");

  fd_set set;

  FD_ZERO (&set);
  FD_SET (fd, &set);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;//1000000;
  
  size = 0;

  if(select(fd+1, &set, NULL, NULL, &timeout) == -1) 
    throw SerialException(strerror(errno));

  if(FD_ISSET(fd, &set))
    size = _read(fd, buf, 1);

  if(size == -1)
    throw SerialException(strerror(errno));

	return size;
}
*/

char *Pentominos::Serial::read(int t, int *size)
  throw(SerialReadException)
{
	if(fd < 0) throw SerialException("Uninitialized!");

  fd_set set;

  FD_ZERO (&set);
  FD_SET (fd, &set);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = t * 1000;
  
  *size = -1;
  memset(buf, 0, sizeof(buf));

  if(select(fd+1, &set, NULL, NULL, &timeout) == -1) 
    throw SerialReadException(strerror(errno));

  if(FD_ISSET(fd, &set)) {
    *size = _read(fd, buf, sizeof(buf));
    
    if(*size == -1)
      throw SerialReadException(strerror(errno));
  }

	return buf;
}

static ssize_t _write(int fd, const void *buf, size_t count) { return write(fd, buf, count); }
void Pentominos::Serial::write(char *buf, size_t size)
{
  int s;
  if(fd < 0) throw SerialException("Device is not active.");

  s = _write(fd, buf, size);

  if(s == -1)
    throw SerialException(strerror(errno));

  //  return size;
}
















/*
std::string Serial::read_barcode()
{
	std::string input;

	if(fd < 0) throw SerialException("Uninitialized!");

	char buf[512];
	memset(buf, 0, sizeof(buf));

	int size = 0;
	int total = 0;
  fd_set set;

  FD_ZERO (&set);
  FD_SET (fd, &set);

  if(select(fd+1, &set, NULL, NULL, NULL) != 1) 
    throw SerialException(strerror(errno));

  if(FD_ISSET(fd, &set)) {
    do {
      size = read(fd, buf + total, sizeof(buf) - total);
      total += size;
    } while((size == BLOCK_SIZE || total == 0) && total < (int)sizeof(buf));
    if(buf[total-1] < 32 || buf[total] > '~') buf[total-1] = '\0';
  }

	input = buf;

	return input;
}
*/
