/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            libartefact_private.h
 *
 *  Thu Feb 11 11:06:37 CET 2010
 *  Copyright 2010 Bent Bisballe Nyeng
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
#ifndef __PENTOMINOS_LIBARTEFACT_PRIVATE_H__
#define __PENTOMINOS_LIBARTEFACT_PRIVATE_H__

#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

#include "libartefact.h"

struct _atf_handle_t {
  atf_error_t error;
  CURL *ch;
};

struct _atf_connection_t {
  atf_handle_t *handle;
  std::string host;
  unsigned short int port;
  bool use_https;
};

struct _atf_data_t {
  std::string devicetype;
  std::string deviceid;
  std::string location;
  std::string operatorid;
  time_t timestamp;
  char *data;
  size_t size;
};

struct _atf_query_t {
  std::string classname;
  resultfilter_t resultfilter;
  int timeflags;
  time_t from_time;
  time_t to_time;
};

struct _atf_transaction_t {
  atf_handle_t *handle;
  atf_id nextid;
  std::string patientid;

  std::map<atf_id, struct _atf_data_t> data;
  std::map<atf_id, struct _atf_query_t> queries;
};

struct _atf_result_t {
  atf_handle_t *handle;

  atf_status_t errcode;
  std::string errstring;
  std::string device_type;
  std::string device_id;
  std::string classname;
  time_t timestamp;

  size_t size;
  char *data;
  size_t start;
  size_t stop;
};

struct _atf_reply_t {
  atf_handle_t *handle;
  std::string data;
  std::map<atf_id, std::vector<struct _atf_result_t> > replies;
};

#endif/*__PENTOMINOS_LIBARTEFACT_PRIVATE_H__*/
