/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            libartefact.cc
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
#include "libartefact.h"

#include <map>
#include <string>

#include <string.h>
#include <base64.h>

#include "libartefact_private.h"
#include "responsparser.h"
#include "resultparser.h"

#ifdef __cplusplus
extern "C" {
#endif

atf_handle_t *atf_init()
{
  atf_handle_t *handle = new atf_handle_t;
  handle->ch = curl_easy_init();
  return handle;
}

void atf_close(atf_handle_t *handle)
{
  if(handle) {
    if(handle->ch) curl_easy_cleanup(handle->ch);
    delete handle;
  }
}

// Connection
atf_connection_t *atf_connect(atf_handle_t *handle,
                              const char *host, unsigned short int port,
                              bool use_https)
{
  if(!handle) return NULL;

  atf_connection_t *connection = new atf_connection_t;
  if(!connection) {
    handle->error = ATF_OUT_OF_MEMORY;
    return NULL;
  }

  connection->handle = handle;
  connection->host = host;
  connection->port = port;
  connection->use_https = use_https;

  curl_easy_setopt(handle->ch, CURLOPT_URL, host);
  curl_easy_setopt(handle->ch, CURLOPT_PORT, port);
  curl_easy_setopt(handle->ch, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(handle->ch, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt(handle->ch, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt(handle->ch, CURLOPT_CONNECTTIMEOUT, 15L);
  curl_easy_setopt(handle->ch, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(handle->ch, CURLOPT_USERAGENT, "libartefact");

  return connection;
}

void atf_disconnect(atf_connection_t *connection)
{
  if(connection) delete connection;
}

// Transaction
atf_transaction_t* atf_new_transaction(atf_connection_t *connection,
                                       const char *patientid)
{
  if(!connection) return NULL;

  atf_transaction_t *transaction = new atf_transaction_t;

  if(!transaction) {
    connection->handle->error = ATF_OUT_OF_MEMORY;
    return NULL;
  }
  transaction->patientid = patientid;
  transaction->handle = connection->handle;
  transaction->nextid = 0;

  return transaction;
}

void atf_free_transaction(atf_transaction_t *transaction)
{
  if(transaction) {

    std::map<atf_id, struct _atf_data_t>::iterator i =
      transaction->data.begin();
    while(i != transaction->data.end()) {
      delete[] i->second.data;
      i++;
    }

    delete transaction;
  }
}

static atf_id get_next_id(atf_transaction_t *transaction)
{
  if(!transaction) return -1;
  return transaction->nextid++; 
}

// Data
atf_id atf_add_data(atf_transaction_t *transaction,
                    const char *devicetype,
                    const char *deviceid,
                    time_t timestamp,
                    const char* location,
                    const char* operatorid,
                    const char *data, size_t size)
{
  atf_id id = get_next_id(transaction);

  transaction->data[id].location = location;
  transaction->data[id].operatorid = operatorid;
  transaction->data[id].devicetype = devicetype;
  transaction->data[id].deviceid = deviceid;
  transaction->data[id].timestamp = timestamp;
  char *buf = new char[size];
  memcpy(buf, data, size);
  transaction->data[id].data = buf;
  transaction->data[id].size = size;

  return id;
}

// Query
atf_id atf_add_query(atf_transaction_t *transaction,
                     const char *classname,
                     resultfilter_t resultfilter,
                     int timeflags,
                     time_t from_time,
                     time_t to_time)
{
  atf_id id = get_next_id(transaction);

  transaction->queries[id].classname = classname;
  transaction->queries[id].resultfilter = resultfilter;
  transaction->queries[id].timeflags = timeflags;
  transaction->queries[id].from_time = from_time;
  transaction->queries[id].to_time = to_time;

  return id;
}

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  std::string *str = (std::string*)userp;
  str->append((char*)buffer, size * nmemb);
  return size * nmemb;
}

// Reply
atf_reply_t *atf_commit(atf_transaction_t *transaction, bool pretend)
{
  if(!transaction) return NULL;

  transaction->handle->error = ATF_NO_ERROR;

  atf_reply_t *reply = new atf_reply_t;

  if(!reply) {
    transaction->handle->error = ATF_OUT_OF_MEMORY;
    return NULL;
  }

  reply->handle = transaction->handle;

  std::string now = "0";
  char *cnow;
  if(asprintf(&cnow, "%lu", time(NULL)) != -1) {
    now = cnow;
    free(cnow);
  }
  
  std::string respons;
  curl_easy_setopt(reply->handle->ch, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(reply->handle->ch, CURLOPT_WRITEDATA, &respons);

  std::string request;
  request =  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<artefact xmlns=\"http://www.aasimon.org/pentominos\"\n"
    "          xmlns:pentominos=\"http://www.aasimon.org/pentominos\"\n"
    "          xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "          xsi:schemaLocation=\"http://www.aasimon.org/pentominos "
    "schema.xsd\">\n";

  request += "  <pentominos:entry patientid=\""+transaction->patientid+"\"\n"
    "                    pretend=\""+(pretend?"true":"false")+"\"\n"
    "                    timestamp=\""+now+"\"/>\n";

  std::map<atf_id, struct _atf_data_t>::iterator di =
    transaction->data.begin();
  while(di != transaction->data.end()) {
    atf_id id = di->first;
    char sid[64];
    snprintf(sid, sizeof(sid), "%u", id);
    
    char ts[32];
    snprintf(ts, 32, "%lu", di->second.timestamp);

    request += "  <pentominos:data id=\""+std::string(sid)+"\"\n"
      "                   operator=\""+di->second.operatorid+"\"\n"
      "                   device_type=\""+di->second.devicetype+"\"\n"
      "                   device_id=\""+di->second.deviceid+"\"\n" 
      "                   timestamp=\""+ts+"\"\n"
      "                   location=\""+di->second.location+"\"\n"
      "                   encoding=\"base64\">\n";
    request += Pentominos::base64encode(di->second.data, di->second.size);
    request += "  </pentominos:data>\n";
    di++;
  }

  std::map<atf_id, struct _atf_query_t>::iterator qi =
    transaction->queries.begin();
  while(qi != transaction->queries.end()) {
    atf_id id = qi->first;
    char sid[64];
    snprintf(sid, sizeof(sid), "%u", id);
    
    char ts_from[64];
    snprintf(ts_from, sizeof(ts_from), "%lu", qi->second.from_time);

    char ts_to[64];
    snprintf(ts_to, sizeof(ts_to), "%lu", qi->second.to_time);
    
    std::string filter = "all";
    if(qi->second.resultfilter == FILTER_LATEST) filter = "latest";
    if(qi->second.resultfilter == FILTER_ALL) filter = "all";
    //    if(qi->second.filter == FILTER_EXACT_TIME) filter = "exacttime";

    request += "  <pentominos:query id=\""+std::string(sid)+"\"\n"
      "                    class=\""+qi->second.classname+"\"\n"
      "                    filter=\""+filter+"\"";
    if((qi->second.timeflags & USE_FROM_TIME) == USE_FROM_TIME)
      request += std::string("\n                    from_time=\"")+ts_from+"\"";
    if((qi->second.timeflags & USE_TO_TIME) == USE_TO_TIME)
      request += std::string("\n                    to_time=\"")+ts_to+"\"";
    request += "/>\n";
    
    qi++;
  }

  request += "</artefact>";

  curl_easy_setopt(reply->handle->ch, CURLOPT_POSTFIELDSIZE,
                   (long)request.length());
  curl_easy_setopt(reply->handle->ch, CURLOPT_POSTFIELDS, request.c_str());
  curl_easy_setopt(reply->handle->ch, CURLOPT_POST, 1L);

  struct curl_slist *slist=NULL;
 // Unset 'Expect' header, set by CURLOPT_POSTFIELDS
  slist = curl_slist_append(slist, "Expect:");
  slist = curl_slist_append(slist, "Content-Type: text/xml");
  slist = curl_slist_append(slist, "Connection: keep-alive");
  curl_easy_setopt(reply->handle->ch, CURLOPT_HTTPHEADER, slist);

  CURLcode errornum = curl_easy_perform(reply->handle->ch);
  if(errornum != CURLE_OK) {
    /*
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror(errornum));
    */
  }

  curl_slist_free_all(slist);

  reply->data = respons;

  ResponsParser parser(reply->handle, reply->replies);
  parser.parse(respons.c_str(), respons.length());

  std::map<atf_id, std::vector<struct _atf_result_t> >::iterator i =
    reply->replies.begin();
  while(i != reply->replies.end()) {
    std::vector<struct _atf_result_t>::iterator j = i->second.begin();
    while(j != i->second.end()) {
      size_t start = j->start;
      bool inquote = false;
      while(start < j->stop) {
        if(respons[start] == '"') inquote = !inquote;
        if(respons[start] == '>' && inquote == false) {
          start++;
          break;
        }
        start++;
      }
      j->start = start; // Just in case we need it at some later point.
      
      int sz = j->stop - start;
      if(sz <= 0 || j->stop > respons.length()) continue;
      j->data = new char[sz];
      memcpy(j->data, respons.c_str() + start, sz);
      j->size = sz;

      j++;
    }
    i++;
  }

  return reply;
}

size_t atf_get_reply_raw_size(atf_reply_t *reply)
{
  return reply->data.length();
}

size_t atf_get_reply_raw(atf_reply_t *reply, char *buf, size_t size)
{
  size_t sz = size<reply->data.length()?size:reply->data.length();
  memcpy(buf, reply->data.c_str(), sz);
  return sz;
}

void atf_free_reply(atf_reply_t *reply)
{
  if(!reply) return;
  if(!reply->handle) return;
  reply->handle->error = ATF_NO_ERROR;

  if(reply) delete reply;
}

size_t atf_get_num_results(atf_reply_t *reply, atf_id id)
{
  if(!reply) return 0;
  if(!reply->handle) return 0;
  reply->handle->error = ATF_NO_ERROR;

  return reply->replies[id].size();
}

atf_result_t *atf_get_result(atf_reply_t *reply, atf_id id, size_t idx)
{
  if(!reply) return NULL;
  if(!reply->handle) return NULL;
  reply->handle->error = ATF_NO_ERROR;

  if(idx > reply->replies[id].size() - 1) {
    reply->handle->error = ATF_INVALID_INDEX;
    return NULL;
  }
  return &reply->replies[id][idx];
}

atf_status_t atf_get_result_status(atf_result_t *result,
                                   char *errbuf, size_t errsize)
{
  if(!result) return ATF_STATUS_ERROR;
  if(!result->handle) return ATF_STATUS_ERROR;
  result->handle->error = ATF_NO_ERROR;

  return result->errcode;
}

time_t atf_get_result_timestamp(atf_result_t *result)
{
  if(!result) return ((time_t) -1);
  if(!result->handle) return ((time_t) -1);
  result->handle->error = ATF_NO_ERROR;

  return result->timestamp;
}

void atf_get_result_device_info(atf_result_t *result,
                                char *idbuf, size_t idsize,
                                char *typebuf, size_t typesize,
                                char *classbuf, size_t classsize)
{
  if(!result) return;
  if(!result->handle) return;
  result->handle->error = ATF_NO_ERROR;

  if(idbuf && idsize)
    snprintf(idbuf, idsize, "%s", result->device_id.c_str());
  if(typebuf && typesize)
    snprintf(typebuf, typesize, "%s", result->device_type.c_str());
  if(classbuf && classsize)
    snprintf(classbuf, classsize, "%s", result->classname.c_str());
}


ssize_t atf_get_result_size(atf_result_t *result)
{
  if(!result) return -1;
  if(!result->handle) return -1;
  result->handle->error = ATF_NO_ERROR;

  return result->size;
}

ssize_t atf_get_result_data(atf_result_t *result,
                            char *buf, size_t maxsize)
{
  if(!result) return -1;
  if(!result->handle) return -1;
  result->handle->error = ATF_NO_ERROR;

  if(maxsize < result->size) {
    result->handle->error = ATF_BUFFER_TOO_SMALL;
    return -1;
  }
  memcpy(buf, result->data, result->size);
  return result->size;
}

// Result node handling (DOM)
atf_result_node_t *atf_get_result_node(atf_result_t *result)
{
  if(!result) return NULL;
  if(!result->handle) return NULL;
  result->handle->error = ATF_NO_ERROR;

  ResultParser parser;

  std::string header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

  parser.parse(header.c_str(), header.length()); 
  parser.parse(result->data, result->size);

  return parser.root();
}

void atf_free_result_node(atf_result_node_t *node)
{
  if(!node) return;

  atf_free_result_node(node->child);

  atf_result_node_t *_this = node;

  while(_this) {
    atf_result_node_t *_next = _this->next;
    if(_this->name) free(_this->name);
    if(_this->value) free(_this->value);
    delete _this;
    _this = _next;
  }
}

// Errorhandling
atf_error_t atf_get_last_error(atf_handle_t *handle)
{
  if(!handle) return ATF_HANDLE_IS_NULL;
  return handle->error;
}

char *atf_get_error_string(atf_error_t errorcode,
                           char *buf, size_t maxsize)
{
  switch(errorcode) {
  case ATF_NO_ERROR:
    snprintf(buf, maxsize, "No error.");
    break;

  case ATF_HANDLE_IS_NULL:
    snprintf(buf, maxsize, "Handle not initialised (is NULL).");
    break;

  case ATF_INVALID_ID:
    snprintf(buf, maxsize, "Invalid ID.");
    break;

  case ATF_BUFFER_TOO_SMALL:
    snprintf(buf, maxsize, "Supplied buffer is too small.");
    break;

  default:
    snprintf(buf, maxsize, "Unknown error (errorcode %d).", errorcode);
    break;
  }

  return buf;
}

#ifdef __cplusplus
}
#endif

#ifdef TEST_LIBARTEFACT
//deps: ../lib/base64.cc ../lib/saxparser.cc responsparser.cc ../lib/debug.cc ../lib/log.cc resultparser.cc
//cflags: -I../lib -I.. $(CURL_CFLAGS) $(OPENSSL_CFLAGS) $(EXPAT_CFLAGS)
//libs: $(CURL_LIBS) $(OPENSSL_LIBS) $(EXPAT_LIBS)
#include "test.h"

static inline bool _err(atf_handle_t *h)
{
  return atf_get_last_error(h) != ATF_NO_ERROR;
}

static char buf[512];
static inline char *_errstr(atf_handle_t *h)
{
  return atf_get_error_string(atf_get_last_error(h), buf, sizeof(buf));
}

TEST_BEGIN;

time_t now = time(NULL);

TEST_MSG("Init");

atf_handle_t *h = atf_init();
if(_err(h)) TEST_FATAL(_errstr(h));

bool use_ssl = false;
atf_connection_t *c = atf_connect(h, "localhost", 11108, use_ssl);
if(_err(h)) TEST_FATAL(_errstr(h));

atf_transaction_t *t = atf_new_transaction(c, "0000000000");
if(_err(h)) TEST_FATAL(_errstr(h));

char data[] = "hello world";
size_t size = sizeof(data);
atf_id id1 = atf_add_data(t, "echo", "devid", now,
                          "lokale42", "birger", data, size);
if(_err(h)) TEST_FATAL(_errstr(h));
TEST_EQUAL_INT(id1, 0, "Test for expected ID 0");

atf_id id2 = atf_add_data(t, "echo", "devid", now, 
                          "lokale42", "birger", data, size);
if(_err(h)) TEST_FATAL(_errstr(h));
TEST_EQUAL_INT(id2, 1, "Test for expected ID 1");

atf_id id3 = atf_add_query(t, "echo", FILTER_LATEST, USE_NONE, 0, 0);
if(_err(h)) TEST_FATAL(_errstr(h));
TEST_EQUAL_INT(id3, 2, "Test for expected ID 2");

atf_id id4 = atf_add_query(t, "echo", FILTER_LATEST, USE_NONE, 0, 0);
if(_err(h)) TEST_FATAL(_errstr(h));
TEST_EQUAL_INT(id4, 3, "Test for expected ID 3");

TEST_MSG("Commit");

atf_reply_t *r = atf_commit(t);
if(_err(h)) TEST_FATAL(_errstr(h));

for(int id = 0; id < 4; id++) {

  TEST_EQUAL_INT(atf_get_num_results(r, id), 1, "Exact one result.");
  atf_result_t *res = atf_get_result(r, id, 0);
  TEST_NOTEQUAL(res, NULL, "Did we get a result?");
  
  atf_status_t status;
  ssize_t sz;
  status = atf_get_result_status(res, NULL, 0);
  if(_err(h)) TEST_FATAL(_errstr(h));
  
  sz = atf_get_result_size(res);
  if(_err(h)) TEST_FATAL(_errstr(h));
  TEST_NOTEQUAL_INT(sz, -1, "Test for errors");
  TEST_NOTEQUAL_INT(sz, 0, "Test for expected size > 0");
  
  {
    char *data = new char[sz + 1];
    data[sz] = '\0';
    atf_get_result_data(res, data, sz);
    if(_err(h)) TEST_FATAL(_errstr(h));
    fprintf(stderr, "[%s]", data);
    delete[] data;
  }

  if(id > 1) { // ID 2 and 3 are queries
    atf_result_node_t *root = atf_get_result_node(res);
    if(_err(h)) TEST_FATAL(_errstr(h));
    TEST_NOTEQUAL(root, NULL, "Test if we got a node");

    TEST_NOTEQUAL(root->child, NULL, "Test if root has a child.");
    TEST_EQUAL(root->next, NULL, "Test if root has any siblings.");

    TEST_EQUAL_STR(root->name, "echo", "Test if rootnode is called 'echo'.");

    atf_result_node_t *node = root->child;

    TEST_EQUAL(node->child, NULL, "Test if node has childred.");
    TEST_EQUAL(node->next, NULL, "Test if node has any siblings.");

    TEST_EQUAL_STR(node->name, "value",
                   "Test if child node is called 'value'.");
    TEST_EQUAL_STR(node->value, data, "Test if child node has data.");

    atf_free_result_node(root);
    if(_err(h)) TEST_FATAL(_errstr(h));
  }

}

atf_free_reply(r);
atf_disconnect(c);
atf_close(h);

TEST_END;

#endif/*TEST_LIBARTEFACT*/
