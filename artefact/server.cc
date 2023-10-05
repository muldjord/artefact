/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            server.cc
 *
 *  Tue Oct 17 15:02:30 CEST 2006
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
#include "server.h"

#include <hugin.hpp>

#include <entry.h>
#include <config.h>
#include <tcpsocket.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include <httpd.h>

#include "xmlparser.h"
#include "postgresql.h"
#include "environment.h"
#include "entryvalidator.h"
#include "queryhandler.h"
#include "translater_xml.h"

#include "luadatainterpreter.h"
#include "luadataclassifier.h"

// Demo CERT, DO NOT USE IN PRODUCTION!
constexpr char CERT[] = ""
  "-----BEGIN CERTIFICATE-----\n"
  "MIICFTCCAX6gAwIBAgIBAjANBgkqhkiG9w0BAQUFADBVMRswGQYDVQQKExJBcGFj\n"
  "aGUgSFRUUCBTZXJ2ZXIxIjAgBgNVBAsTGUZvciB0ZXN0aW5nIHB1cnBvc2VzIG9u\n"
  "bHkxEjAQBgNVBAMTCWxvY2FsaG9zdDAeFw0wNzA2MjEwODE4MzZaFw0wODA2MjAw\n"
  "ODE4MzZaMEwxGzAZBgNVBAoTEkFwYWNoZSBIVFRQIFNlcnZlcjEZMBcGA1UECxMQ\n"
  "VGVzdCBDZXJ0aWZpY2F0ZTESMBAGA1UEAxMJbG9jYWxob3N0MIGfMA0GCSqGSIb3\n"
  "DQEBAQUAA4GNADCBiQKBgQDWTACKSoxd5cL06w7RtPIhFqY1l3UE/aRGmPmh8gEo\n"
  "w3zNf+gWxco2yjQgBTQhGww1ybOsAUtXPIsUOSFAGvPUKJZf8ibZMiJEzl2919uz\n"
  "IcV9+cUm7k3jFPQx4ALQEalbV++o/lfT5lhgsSiH1t1eln2omVrGCjI/1HeYrw7X\n"
  "owIDAQABMA0GCSqGSIb3DQEBBQUAA4GBALVFzprK6rYkWVZZZwq85w2lCYJpEl9a\n"
  "66IMzIwNNRfyZMoc9D9PSwsXKYfYOg1RpMt7RhWT/bpggGlsFqctsAgJSv8Ol5Cz\n"
  "DqTXhpV+8WOG6l4xDYZz3U3ajiu2jth2+aaMuWKy9Wkr8bzHGDufltToLalucne2\n"
  "npM7yCJ83Ana\n"
  "-----END CERTIFICATE-----";

// Demo KEY, DO NOT USE IN PRODUCTION!
constexpr char KEY[] = ""
  "-----BEGIN RSA PRIVATE KEY-----\n"
  "MIICXAIBAAKBgQDWTACKSoxd5cL06w7RtPIhFqY1l3UE/aRGmPmh8gEow3zNf+gW\n"
  "xco2yjQgBTQhGww1ybOsAUtXPIsUOSFAGvPUKJZf8ibZMiJEzl2919uzIcV9+cUm\n"
  "7k3jFPQx4ALQEalbV++o/lfT5lhgsSiH1t1eln2omVrGCjI/1HeYrw7XowIDAQAB\n"
  "AoGANUXHjJljs6P+hyw4DuHQn3El+ISiTo9PW02EIUIsD5opWFzHsYGR93Tk6GDi\n"
  "yKgUrPprdAMOW61tVaWuImWQ32R2xyrJogjGYo9XE2xAej9N37jM0AGBtn/vd4Dr\n"
  "LsYfpjNaM3gqIChD73iYfO+CrNbdLqTxIdG53g/u05GJ4cECQQD0vMm5+a8N82Jb\n"
  "oHJgE2jb83WqaYBHe0O03ujtiq3+hPZHoVV3iJWmA/aMlgdtunkJT3PdEsVfQNkH\n"
  "fvzR9JhbAkEA4CiZRk5Gcz7cEqyogDTMQYtmrE8hbgofISLuz1rpTEzd8hFAcerU\n"
  "nuwFIT3go3hO7oIHMlKU1H5iT1BsFvegWQJBAOSa6A+5A+STIKAX+l52Iu+5tYKN\n"
  "885RfMgZpBgm/yoMxwPX1r7GLYsajpV5mszLbz3cIo0xeH3mVBOlccEoqZsCQECP\n"
  "8PWq/eebp09Jo46pplsKh5wBfqNvDuBAa4AVszRiv1pFVcZ52JudZyzX4aezsyhH\n"
  "E0OPPYamkDI/+6Hx2KECQHF9xV1XatyXuFmfRAInK2BtfGY5UIvJaLxVD3Z1+i6q\n"
  "/enz7/wUwvC6G4FSWNMYgAYJOfwZ3BerdkqcRNxyR/Q=\n"
  "-----END RSA PRIVATE KEY-----";

static Tree handleData(Pentominos::Entry &entry,
                       Pentominos::Data &data,
                       PostgreSQLTransaction *pgsql)
{
  if(entry.patientid == "") {
    return statusTree(data.id, STATUS_ERROR, "Empty patientid.");
  }

  if(pgsql == NULL) {
    return statusTree(data.id, STATUS_ERROR, "Database connection error.");
  }

  try {
    LUADataClassifier ldc;
    data.classlist = ldc.classify(data);
    pgsql->post(entry, data);
  } catch(PostgreSQLException &e) {
    ERR(server,
        "Error during postgresql connection/writing! :\n%s\n",
        e.what());
    return statusTree(data.id, STATUS_ERROR, e.what());
  } catch(LUADataClassifierException &e) {
    ERR(server,
        "Error during classification :\n%s\n",
        e.what());
    return statusTree(data.id, STATUS_ERROR, e.what());
  }

  return statusTree(data.id, STATUS_OK, "Data Success");
}

static Trees handleQuery(Pentominos::Entry &entry,
                         Pentominos::Query &query,
                         PostgreSQLTransaction *pgsql,
                         bool resume)
{
  if(entry.patientid == "") {
    Trees ts;
    ts.push_back(statusTree(query.id, STATUS_ERROR, "Empty patientid."));
    return ts;
  }

  if(pgsql == NULL) {
    Trees ts;
    ts.push_back(statusTree(query.id, STATUS_ERROR,
                            "Database connection error."));
    return ts;
  }

  try {
     
    QueryHandler queryhandler(entry, *pgsql, resume);
    return queryhandler.handle(query);

  } catch(PostgreSQLException &e) {
    ERR(server, "Error during postgresql reading: %s", e.what());
  } catch(QueryHandlerException &e) {
    ERR(server, "Error during query handling: %s", e.what());
  } catch(LUADataClassifierException &e) {
    ERR(server, "Error during classification: %s", e.what());
  } catch(LUADataInterpreterException &e) {
    ERR(server, "Error during interpretation: %s", e.what());
  } catch( ... ) {
    ERR(server, "Other exception\n");
  }

  Trees ts;
  ts.push_back(statusTree(query.id, STATUS_ERROR, "ERROR!"));
  return ts;
}

static std::string entryHandler(Pentominos::Entry &entry,
                                PostgreSQL &pgsql)
{
  std::string result;
  EntryValidator validator(entry);

  PostgreSQLTransaction *t = pgsql.transaction(entry.pretend);

  try {
    Trees trees;

    // Save files to correct locations.
    validator.fixate();
  
    // For each Data in Entry->data handleData(...)
    std::vector<Pentominos::Data*>::iterator di = entry.datalist.begin();
    while(di != entry.datalist.end()) {
      trees.push_back(handleData(entry, **di, t));
      di++;
    }

    // For each Query in Entry->queries handleQuery(...)
    std::vector<Pentominos::Query*>::iterator qi = entry.querylist.begin();
    while(qi != entry.querylist.end()) {
      Trees ts = handleQuery(entry, **qi, t, entry.pretend);
      trees.insert(trees.end(), ts.begin(), ts.end());
      qi++;
    }
  
    result = translate_xml(trees);
    DEBUG(server, "RESULT (latest):\n%s\n", result.c_str());
  } catch(...) {
    ERR(server, "entryHandler terminated due to exception thrown...\n");
  }

  if(entry.pretend) validator.erase();
  delete t;

  DEBUG(server, "...done with connection\n");
  return result;
}

static std::string connectionHandler(Environment &env, const std::string &xml)
{
  DEBUG(server, "============= Got new connection =============\n");

  std::string reply;

  AutoBorrower<PostgreSQL *> borrower(env.dbpool);
  PostgreSQL *db = borrower.get();

  DEBUG(server, "Got XML: %s\n", xml.c_str());

  try {
    DEBUG(server, "New XMLParser\n");
    XMLParser parser;
    
    if(parser.parse(xml.c_str(), xml.length())) {
      reply = entryHandler(parser.entry, *db);
    }
    
  } catch (XMLParserException &e) {
    ERR(server, "Error during parsing:\n%s\n",  e.what());
  } catch( Pentominos::Exception &e ) {
    ERR(server, "Error during parsing:\n%s\n", e.what());
  }

  INFO(server, "OUT OF READ LOOP\n");

  return reply;
}


class ArtefactHttpd : public Httpd {
public:
  void error(const std::string &err)
  {
    ERR(server, "Httpd server error: %s\n", err.c_str());
  }

  void *begin(const std::string &url,
              const std::string &method,
              const std::string &version,
              headers_t &headers)
  {
    DEBUG(server, "Connection initiated: %s\n", url.c_str());
    std::string *xml = new std::string;
    return xml;
  }

  bool data(void *ptr, const char *data, unsigned int data_size)
  {
    std::string *xml = (std::string*)ptr;
    xml->append(data, data_size);
    return true;
  }

  bool complete(void *ptr, Httpd::Reply &reply)
  {
    std::string *xml = (std::string*)ptr;
    
    reply.data = connectionHandler(env, *xml);
    reply.headers["Content-Type"] = "text/plain; charset=UTF-8";
    reply.status = 200; // http 'OK'
    
    DEBUG(server, "Connection done\n");

    return true;
  }

  void cleanup(void *ptr)
  {
    std::string *xml = (std::string*)ptr;
    delete xml;
  }

private:
  Environment env;
};

extern bool pentominos_is_running;
void server()
{
  int port;
  try {
    port = Pentominos::config["port"];
  } catch( ... ) {
    ERR(server, "Could not read port.");
    return;
  }

  int maxconn;
  try {
    maxconn = Pentominos::config["maxconn"];
  } catch( ... ) {
    ERR(server,
        "Could not read maximum number of connections (maxconn).");
    return;
  }

  bool usessl = false;
  try {
    usessl = Pentominos::config["use_ssl"];
  } catch( ... ) {
    ERR(server,
        "Could not read ssl boolean (use_ssl).");
    return;
  }

  DEBUG(server, "Server start, port %d, pid: %d, ssl: %s\n", port, getpid(), usessl?"true":"false");
  std::string sslkey = KEY;
  std::string sslcert = CERT;

  if(usessl) {
    try {
      std::string s = Pentominos::config["ssl_key"];
      std::ifstream f(s);
      if(f.is_open()) {
        std::stringstream buffer;
        buffer << f.rdbuf();
        f.close();
        s = buffer.str();
      }
      sslkey = s;
    } catch( ... ) {
      DEBUG(server, "'ssl_key' not found in config, falling back to hardcoded default. Please consider changing this.\n");
    }

    try {
      std::string s = Pentominos::config["ssl_cert"];
      std::ifstream f(s);
      if(f.is_open()) {
        std::stringstream buffer;
        buffer << f.rdbuf();
        f.close();
        s = buffer.str();
      }
      sslcert = s;
    } catch( ... ) {
      DEBUG(server, "'ssl_cert' not found in config, falling back to hardcoded default. Please consider changing this.\n");
    }
  }
  
  ArtefactHttpd httpd;
  if(usessl) {
    httpd.listen_ssl(port,
                     sslkey,
                     sslcert,
                     maxconn);
  } else {
    httpd.listen(port,
                 maxconn);
  }
  /*
  */

  while(pentominos_is_running) sleep(1);

  httpd.stop();

  while(httpd.is_running()) sleep(1);

  DEBUG(server, "Server gracefully shut down.\n");
}
