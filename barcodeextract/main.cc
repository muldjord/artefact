#include <vector>
#include <time.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <string.h>

#include <libartefact.h>

#include "pgwork.h"

#include <hugin.hpp>

class barcodeinfo_t {
public:
  std::string vendor;
  std::string model;
  std::string strength;
  std::string serial;
  std::string barcode;
  std::string time;
  std::string cpr;
};
typedef std::list<barcodeinfo_t> BarcodeList;

std::string barcode2str(barcodeinfo_t b) {
  return b.vendor + ", " +
         b.model + ", " +
         b.strength + ", " +
         b.serial + ", " +
         b.barcode + ", " +
         b.time + ", " + 
         b.cpr;
}

int main(int argc, char* argv[])
{
  std::string fromtime = "0";
  std::string totime = "9";
  bool pretendonly = true;
  std::string host = "localhost";
  std::string port = "5432";
  std::string filepath = "";
  std::string artefacthost = "localhost";
  unsigned int artefactport = 11108;

  int c = 0;
  while ((c = getopt(argc, argv, "a:A:st:T:?h:p:f:")) != -1) {
    switch(c) {
      case 'a':
        artefacthost = strdup(optarg);
        break;
      case 'A':
        artefactport = atoi(optarg);
      case 'f':
        filepath = strdup(optarg);
        break;
      case 'h':
        host = strdup(optarg);
        break;
      case 'p':
        port = strdup(optarg);
        break;
      case 's':
        pretendonly = false;
        break;
      case 't':
        fromtime = strdup(optarg);
        break;
      case 'T':
        totime = strdup(optarg);
        break;
      case '?':
      default:
        printf("Usage: barextract [-s] [-h <dbhost>] [-p <dbport>] [-t <timestamp>] [-T <timestamp>] [-a <artefacthost>] [-A <artefactport>]\n"
               "         -a    Artefact Host\n"
               "         -A    Artefact port\n"
               "         -f    Store errors in files in this directory\n"
               "         -h    Db host\n"
               "         -p    Db port\n"
               "         -s    Save/store entries into artefact db\n"
               "         -t    Only extract entries newer than this time\n"
               "         -T    Only extract entries older (included) than this time\n");
        return 0;
        break;
    }
  }

  std::string conndbstr = "host=" + host + " port=" + port + " user=pentominos";
  DEBUG(main, "Connection info: '%s'\n", conndbstr.c_str());
  PGconn* pgconn = PQconnectdb(conndbstr.c_str());

  Work w(pgconn);

  std::string sqlstatement = "SELECT vendor,model,strength,serial,barcode,timestamp,cpr "
                             "FROM barcode WHERE timestamp > ";
  sqlstatement += fromtime + " AND timestamp <= ";
  sqlstatement += totime + ";";

  DEBUG(main, "Executing statetement: %s\n", sqlstatement.c_str());
  result_t r = w.exec(sqlstatement.c_str()); 

  DEBUG(main, "Got %zu results.\n", r.size());

  BarcodeList barcodes;
  for(result_t::iterator it_row = r.begin(); it_row != r.end(); it_row++) {
    tuple_t t = *it_row;

    barcodeinfo_t b;
    b.vendor = t.at(0);
    b.model = t.at(1);
    b.strength = t.at(2);
    b.serial = t.at(3);
    b.barcode = t.at(4);
    b.time = t.at(5);
    b.cpr = t.at(6);

    barcodes.push_back(b);
  }

  atf_handle_t* handle = atf_init();
  atf_connection_t* conn = atf_connect(handle, artefacthost.c_str(), artefactport, false);

  for(BarcodeList::iterator it = barcodes.begin();
      it != barcodes.end(); it++) {
    barcodeinfo_t b = *it;

    if(b.cpr.empty()) {
      WARN(main, "Empty cpr - Skipping entry!!!\n");
      continue; 
    }

    // Interpret data
    atf_transaction_t* transaction = atf_new_transaction(conn, b.cpr.c_str());
    
    std::string data = b.barcode;
    if(!b.serial.empty()) data += "\n" + b.serial;

    atf_add_data(transaction, "barcode", "OLD_BARCODE", atoll(b.time.c_str()), "OLD_BARCODE_DB", "", data.c_str(), data.size());
    atf_id queryid = atf_add_query(transaction, "barcode", FILTER_LATEST, USE_NONE, 0, 0);
//    atf_reply_t* reply = atf_commit(transaction, true);
    atf_reply_t* reply = atf_commit(transaction, pretendonly);
    
    // Get result
    size_t errsize = 0;
    atf_result_t* result = atf_get_result(reply, queryid, errsize);
   
    bool errflag = false;
    atf_result_node_t* group = atf_get_result_node(result);
    if(group && !strcmp(group->name, "barcode") &&
       group->child) {
      atf_result_node_t* values = group->child;
      do {
        if(!strcmp(values->name, "vendor")) {
          if(strcmp(b.vendor.c_str(), values->value)) errflag = true;
        }
        if(!strcmp(values->name, "model")) {
          if(strcmp(b.model.c_str(), values->value)) errflag = true;
        }
        if(!strcmp(values->name, "power")) {
          if(strcmp(b.strength.c_str(), values->value)) errflag = true;
        }
        if(!strcmp(values->name, "serial")) {
          if(strcmp(b.serial.c_str(), values->value)) errflag = true;
        }
        if(!strcmp(values->name, "rawdata")) {
          char buf[4096];
          memset(buf, 0, 4096);
          strcat(buf, b.barcode.c_str());
          strcat(buf, "\n");
          strcat(buf, b.serial.c_str());
          if(strcmp(buf, values->value)) errflag = true;
        }
      }
      while( (values = values->next) );
      atf_free_result_node(values);
    }
    else {
      errflag = true;
    }

    if(errflag) {
      WARN(dataerr, "=============================\n");
      WARN(dataerr, "%s\n", barcode2str(b).c_str());
      char buf_res[4096];
      memset(buf_res, 0, sizeof(buf_res));
      atf_get_result_data(result, buf_res, 4096);
      WARN(dataerr, "\"%s\"\n", buf_res);
      char buf_reply[4096];
      memset(buf_reply, 0, sizeof(buf_reply));
      atf_get_reply_raw(reply, buf_reply, 4096);
      WARN(dataerr, "\"%s\"\n", buf_reply);
      if(!filepath.empty()) {
        FILE* f = fopen(std::string(filepath + "/" + fromtime + "-" + totime).c_str(), "a");
        fputs("=============================\n", f);
        fputs(barcode2str(b).c_str(), f);
        fputc('\n', f);
        fputs(buf_res, f);
        fputc('\n', f);
        fputs(buf_reply, f);
        fputc('\n', f);
        fclose(f);
      }
    }
    else {
      DEBUG(main, "SUCCESS! %s\n", barcode2str(b).c_str());
    }

    atf_free_result_node(group);

    // Cleanup
    atf_free_reply(reply);
    atf_free_transaction(transaction);

    usleep(100000);
  }

  atf_disconnect(conn);
  atf_close(handle);

  DEBUG(main, "Extraction done!");
}
