/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            mkdirr.cc
 *
 *  Mon Mar 19 14:30:29 CET 2007
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
#include "mkdirr.h"

#include <hugin.hpp>

// For mkdir
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#include <string.h>

#include <stdio.h>

static int _mkdir(const char *dir) {
  char tmp[FILENAME_MAX];
  char *p = NULL;
  size_t len;
  
  snprintf(tmp, sizeof(tmp), "%s", dir);
  len = strlen(tmp);

  if(tmp[len - 1] == '/') tmp[len - 1] = 0;

  for(p = tmp + 1; *p; p++) {
    if(*p == '/') {
      *p = 0;
      int ret = mkdir(tmp, S_IRWXU);
      if(ret != 0 && errno != EEXIST) return ret;
      *p = '/';
    }
  }

  int ret = mkdir(tmp, S_IRWXU);
  return ret;
}
int Pentominos::mkdirr(std::string path)
{
  int ret = _mkdir(path.c_str());

  if(ret != 0) {
    //printf("errno: %d\n", errno);
    switch(errno) {
    case EEXIST: // 13
      ///printf("EEXIST\n");
      break;
    case EACCES: // 17
      DEBUG(mkdirr, "Error recursively creating path: %s\n", strerror(errno));
      //throw Pentominos::MkdirRStatException(path, strerror(errno));
      throw Pentominos::MkdirRDirectoryException(path, strerror(errno));
      break;
    case EFAULT: // 14
      DEBUG(mkdirr, "Error recursively creating path: %s\n", strerror(errno));
      throw Pentominos::MkdirRDirectoryException(path, strerror(errno));
      break;
    case EPERM: // 
      // printf("EPERM\n");
      break;
    default:
      throw Pentominos::MkdirRDirectoryException(path, strerror(errno));
      //printf("mkdir errno: %s\n", strerror(errno));
    }
  }

  return 0;
}

std::string Pentominos::stripFilename(std::string filename)
{
  return filename.substr(0, filename.rfind("/"));
}
