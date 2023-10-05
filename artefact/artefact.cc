/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            artefact.cc
 *
 *  Mon Oct  9 18:06:20 CEST 2006
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
#include <hugin.hpp>

// For ETC
#include <sysconfig.h>

// For setuid and setgid
#include <sys/types.h>
#include <unistd.h>

// For signal
#include <signal.h>

// For errno and strerror
#include <errno.h>

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

// For getpwent and getgrent
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

// For getopt_long and friends
#include <getopt.h>

#include <config.h>

#include <daemon.h>
#include "server.h"

#include "tcpsocket.h"

static const char version_str[] =
"Pentominos Dataserver - Artefact v" VERSION "\n"
;

static const char copyright_str[] =
"Copyright (C) 2006-2007 Bent Bisballe Nyeng - Aasimon.org.\n"
"This is free software.  You may redistribute copies of it under the terms of\n"
"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
"There is NO WARRANTY, to the extent permitted by law.\n"
"\n"
"Written by Bent Bisballe Nyeng (deva@aasimon.org)\n"
;

static const char usage_str[] =
"Usage: %s [options]\n"
"Options:\n"
"  -f, --foreground    Run in foreground mode (non-daemon mode)\n"
"  -u, --user user     Run as 'user' (overrides the configfile)\n"
"  -g, --group group   Run as 'group' (overrides the configfile)\n"
"  -l, --lua-basedir d Use 'd' as basedir for finding the lua scripts (default " LUA_BASEDIR ").\n"
"  -v, --version       Print version information and exit.\n"
"  -h, --help          Print this message and exit.\n"
"  -D, --debug ddd     Enable debug messages on 'ddd'; see documentation for details\n"
"  -L, --logfile file  Write output to file, instead of stderr.\n"
"  -P, --pidfile file  Write pid of the running daemon to file.\n"
;

std::string lua_basedir;// = LUA_BASEDIR;
std::vector< std::string > static_classes;

//static std::string logfile;
//static FILE *logfp = stderr;

class ArtefactDaemon : public Daemon {
  int daemon_main() {
    server();
    return 0;
  };
};

bool pentominos_is_running = true;
void ctrl_c(int)
{
  DEBUG(sig, "Ctrl+c detected\n");
  pentominos_is_running = false;
}

void reload(int)
{
  printf("Reopening logfile...\n");
  if(hugin_reopen_log() != HUG_STATUS_OK) {
    fprintf(stderr, "Could not reopen logfile!\n");
    return;
  }
  DEBUG(artefact, "Reopened log\n");
}

double timing_mult = 0;

long long getCPUTick() {
  long long s64Ret;
  __asm__ __volatile__ ("rdtsc" : "=A"(s64Ret):);
  return s64Ret;
}

int main(int argc, char *argv[])
{
  const char *hugin_filter = "+all";
  const char *logfile = NULL;
  const char *sysloghost = NULL;
  int c;
  std::string configfile = ETC"/artefact.conf";
  char *user = NULL;
  char *group = NULL;
  bool foreground = false;
  std::string pidfile;

  unsigned int hugin_flags = 0;

  int option_index = 0;
  while(1) {
    //    int this_option_optind = optind ? optind : 1;
    static struct option long_options[] = {
      {"foreground", no_argument, 0, 'f'},
      {"config", required_argument, 0, 'c'},
      {"user", required_argument, 0, 'u'},
      {"group", required_argument, 0, 'g'},
      {"lua-basedir", required_argument, 0, 'l'},
      {"help", no_argument, 0, 'h'},
      {"version", no_argument, 0, 'v'},
      {"debug", required_argument, 0, 'D'},
      {"pidfile", required_argument, 0, 'P'},
      {"logfile", required_argument, 0, 'L'},
      {"sysloghost", required_argument, 0, 'S'},
      {0, 0, 0, 0}
    };
    
    c = getopt_long (argc, argv, "hvfc:u:g:l:D:P:L:S:",
                     long_options, &option_index);
    
    if (c == -1)
      break;

    switch(c) {
    case 'L':
      hugin_flags |= HUG_FLAG_OUTPUT_TO_FILE;
      logfile = strdup(optarg);
      break;

    case 'S':
      hugin_flags |= HUG_FLAG_OUTPUT_TO_SYSLOG;
      sysloghost = strdup(optarg);
      break;

    case 'c':
      configfile = optarg;
      break;

    case 'f':
      foreground = true;
      break;

    case 'u':
      user = strdup(optarg);
      break;

    case 'g':
      group = strdup(optarg);
      break;

    case 'l':
      lua_basedir = optarg;
      break;

    case 'D':
      hugin_flags |= HUG_FLAG_USE_FILTER;
      hugin_filter = optarg;
      break;

    case 'P':
      pidfile = optarg;
      break;

    case '?':
    case 'h':
      printf("%s", version_str);
      printf(usage_str, argv[0]);
      return 0;

    case 'v':
      printf("%s", version_str);
      printf("%s", copyright_str);
      return 0;

    default:
      break;
    }
  }


  // Check if config file exists and is readable:
  FILE *cfg = fopen(configfile.c_str(), "r");
  if(!cfg) {
    ERR(artefact, "Cannot open config file for reading: %s\n",
        configfile.c_str());
    return 1;
  }
  fclose(cfg);

  Pentominos::parse_config(configfile);

  DEBUG(main, "Reading static classes.\n");

  Pentominos::VariantList vl = Pentominos::config["static_classes"];
  Pentominos::VariantList::iterator i = vl.begin();
  while(i != vl.end()) {
    static_classes.push_back(*i);
    i++;
  }

  DEBUG(main, "Setting user/group.\n");

  if(!user) {
    try {
      std::string userstr = Pentominos::config["user"];
      user = strdup(userstr.c_str());
    } catch( ... ) {
      WARN(artefact, "User not found in config.\n");
    }
  }

  if(!group) {
    try {
      std::string groupstr = Pentominos::config["group"];
      group = strdup(groupstr.c_str());
    } catch( ... ) {
      WARN(artefact, "Group not found in config.\n");
    }
  }

  if(lua_basedir == "") {
    try {
      std::string s = Pentominos::config["lua_basedir"];
      lua_basedir = s;
    } catch( ... ) {
      lua_basedir = LUA_BASEDIR;
    }
  }

  if(!sysloghost) {
    try {
      std::string s = Pentominos::config["sysloghost"];
      sysloghost = strdup(s.c_str());
    } catch( ... ) {
      // then dont log to syslog 
    }
  }

  if((logfile == NULL) && (sysloghost == NULL)) {
    hugin_flags |= HUG_FLAG_OUTPUT_TO_STDOUT;
  }

  hug_status_t status = hug_init(hugin_flags,
                                 HUG_OPTION_FILTER, hugin_filter,
                                 HUG_OPTION_FILENAME, logfile,
                                 HUG_OPTION_SYSLOG_HOST, sysloghost,
                                 HUG_OPTION_SYSLOG_PORT, 514,
                                 HUG_OPTION_END);

  if(status != HUG_STATUS_OK) {
    printf("Error: %d\n", status);
    return 1;
  }

  { // Calibrate tick timing...
    double len = 100000;
    long long start = getCPUTick();
    usleep(len);
    long long end = getCPUTick();
    timing_mult = (end - start) / len;
    DEBUG(timing, "1 usec == %f ticks\n", timing_mult);
  }


  signal(SIGHUP, reload);
  signal(SIGINT, ctrl_c);

  ArtefactDaemon daemon;

  DEBUG(main, "Running daemon.\n");

  daemon.run(user, group, !foreground, pidfile);

  // Clean up
  if(user) free(user);
  if(group) free(group);

  //  if(logfp != stderr) fclose(logfp);

  hug_close();

  return 0;
}
