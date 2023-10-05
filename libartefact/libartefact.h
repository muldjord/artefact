/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            libartefact.h
 *
 *  Thu Nov  5 09:28:44 CET 2009
 *  Copyright 2009 Bent Bisballe Nyeng
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
#ifndef __PENTOMINOS_LIBARTEFACT_H__
#define __PENTOMINOS_LIBARTEFACT_H__

#include <time.h>
#include <stdlib.h>
#include <libartefactstatus.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _atf_handle_t;
struct _atf_connection_t;
struct _atf_transaction_t;
struct _atf_datalist_t;
struct _atf_querylist_t;
struct _atf_reply_t;
struct _atf_result_t;

typedef struct _atf_handle_t atf_handle_t;
typedef struct _atf_connection_t atf_connection_t;
typedef struct _atf_transaction_t atf_transaction_t;
typedef int atf_id;
typedef struct _atf_reply_t atf_reply_t;
typedef struct _atf_result_t atf_result_t;

/**
 * libArtefact DOM stucture.
 * 'name' and 'value' contains the name/value pair of the current node.
 * 'next' contains the next node in the same level, or NULL if there
 *  are no more nodes.
 * 'child' contains a pointer to the first child of this node or NULL if
 *  it has no children.
 */
typedef struct _atf_result_node_t {
  char *name;
  char *value;
  struct _atf_result_node_t* next;
  struct _atf_result_node_t* child;
} atf_result_node_t;

typedef enum {
  ATF_NO_ERROR = 0,
  ATF_OUT_OF_MEMORY = 1000,
  ATF_HANDLE_IS_NULL = 2000,
  ATF_INVALID_ID = 3000,
  ATF_BUFFER_TOO_SMALL = 4000,
  ATF_INVALID_INDEX = 5000,
  ATF_UNKNOWN_ERROR = 0xffffffff
} atf_error_t;

// Lib
atf_handle_t *atf_init(); // if NULL := "out of memory"

void atf_close(atf_handle_t *handle);

// Connection
atf_connection_t *atf_connect(atf_handle_t *handle,
                              const char *host, unsigned short int port,
                              bool use_https);

void atf_disconnect(atf_connection_t *connection);

// Transaction
atf_transaction_t* atf_new_transaction(atf_connection_t *connection,
                                       const char *patientid);

void atf_free_transaction(atf_transaction_t *transaction);

atf_reply_t *atf_commit(atf_transaction_t *transaction, bool pretend = false);

// Data
atf_id atf_add_data(atf_transaction_t *transaction,
                    const char *devicetype,
                    const char *deviceid,
                    time_t timestamp,
                    const char* location,
                    const char* operatorid,
                    const char *data, size_t size);

// Query

// Result filter
typedef enum {
  FILTER_ALL    = 0x00, // Get all matching results
  FILTER_LATEST = 0x01  // Get at most one result, being the latest one.
} resultfilter_t;

// Time flags
#define USE_NONE      0x00 // Don't use timestamps
#define USE_FROM_TIME 0x01 // Use from timestamp (unset := -inf)
#define USE_TO_TIME   0x02 // Use to timestamp (unset := inf)
#define USE_BOTH      (USE_FROM_TIME | USE_TO_TIME) // Use both timestamps

atf_id atf_add_query(atf_transaction_t *transaction,
                     const char *classname,
                     resultfilter_t resultfilter,
                     int timeflags,
                     time_t from_time,
                     time_t to_time);

// Reply
size_t atf_get_reply_raw_size(atf_reply_t *reply);
size_t atf_get_reply_raw(atf_reply_t *reply, char *buf, size_t size);

void atf_free_reply(atf_reply_t *reply);

// Result
size_t atf_get_num_results(atf_reply_t *reply, atf_id id);
atf_result_t *atf_get_result(atf_reply_t *reply, atf_id id, size_t idx);

/**
 * buf == NULL or size == 0, ignores string
 */
atf_status_t atf_get_result_status(atf_result_t *result,
                                   char *errbuf, size_t errsize);

time_t atf_get_result_timestamp(atf_result_t *result);

/**
 * buf == NULL or size == 0, ignores string
 */
void atf_get_result_device_info(atf_result_t *result,
                                char *idbuf, size_t idsize,
                                char *typebuf, size_t typesize,
                                char *classbuf, size_t classsize);

ssize_t atf_get_result_size(atf_result_t *result);

ssize_t atf_get_result_data(atf_result_t *result,
                            char *buf, size_t maxsize);

// Result node handling (DOM)
atf_result_node_t *atf_get_result_node(atf_result_t *result);

void atf_free_result_node(atf_result_node_t *node);

// Errorhandling
atf_error_t atf_get_last_error(atf_handle_t *handle);

char *atf_get_error_string(atf_error_t errorcode,
                           char *buf, size_t maxsize);

#ifdef __cplusplus
}
#endif

#endif/*__PENTOMINOS_LIBARTEFACT_H__*/
