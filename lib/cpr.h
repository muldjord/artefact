/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            cpr.h
 *
 *  Wed Feb 21 12:01:21 CET 2007
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
#ifndef __ARTEFACT_CPR_H__
#define __ARTEFACT_CPR_H__

#include <string>

#include "exception.h"

namespace Pentominos {

  /**
   * The CPRConnectionException is thrown by CPR if the cpr server cannot be reached.
   */
  class CPRConnectionException: public Pentominos::Exception {
    public:
    CPRConnectionException(std::string reason) : 
      Pentominos::Exception("Error while connecting to CPR server: " + reason) {}
  };

  /**
   * The CPRNotFoundException is thrown by CPR if the requested cpr cannot be found
   * on the cpr server.
   */
  class CPRNotFoundException: public Pentominos::Exception {
    public:
    CPRNotFoundException() : 
      Pentominos::Exception("Could not find CPR in db.") {}
  };

  /**
   * The CPRMalformedException is thrown by CPR if the cpr server rejects the requested
   * cpr. This is most likely because the cpr does not pass the modulo 11 test.
   */
  class CPRMalformedException: public Pentominos::Exception {
    public:
    CPRMalformedException() : 
      Pentominos::Exception("CPR is malformed.") {}
  };
  
  /**
   * Sex type (male/female)
   */
  typedef enum {
    sex_unknown = 0x00, ///< Sex is unknown (probably not set)
    sex_male    = 0x01, ///< Sex is male
    sex_female  = 0x02  ///< Sex is female
  } sex_t;
  
  /**
   * This class is used to make cpr lookups and gain the results in a handy
   * fashion.
   */
  class CPR {
  public:
    /**
     * The constuctor establishes a connection to the cpr server, and parses
     * the results.\n
     * The constructor might throw the following exceptions: CPRConnectionException,
     * CPRNotFoundException or CPRMalformedException.
     * @param cpr The cpr to look up.
     * @param server The server to connect to.
     * @param port The port to connect to.
     */
    CPR(std::string cpr, std::string server = "server.dns.addr",
        unsigned short int port = 10301);
    
    /**
     * Find out if the communication with the server went well and a valid cpr
     * has been loaded and parsed into the datafields.
     */
    bool valid();
    
    std::string cpr(); ///< @return The cpr number of the person loaded.
    std::string efternavn(); ///< @return The surname of the person loaded.
    std::string fornavn(); ///< @return The firstname(s) of the person loaded.
    std::string conavn(); ///< @return The co name of the person loaded.
    std::string lokalitet(); ///< @return Locality
    std::string adresse(); ///< @return Address.
    std::string bynavn(); ///< @return Cityname
    std::string postnr(); ///< @return Postal code
    std::string postby(); ///< @return Postal city
    std::string kommunenr(); ///< @return Kommune number
    std::string sikringsgruppe(); ///< @return Assurance group
    std::string laegenr(); ///< @return Doctor number
    std::string beskyttelse(); ///< @return Protected.
    std::string reserveret(); ///< @return Reserved.
    std::string aktion(); ///< @return No idea what this is!!!
    std::string doedsdato(); ///< @return Time of death (if dead!)
    std::string insert_at(); ///< @return Insertion date.
    std::string moved_at(); ///< @return Moving time.
    std::string num_fields(); ///< @return Number of fields
    
    // Parsed fields
    int birthday(); ///< @return Parsed day of birth (from cpr)
    int birthmonth(); ///< @return Parsed month of birth (from cpr)
    int birthyear(); ///< @return Parsed year of birth (from cpr) 
                     ///< according to journal number 2001/6201-2 from CPR-KONTORET
                     ///< and http://www.cpr.dk/cpr_artikler/Files/Fil1/4258.doc
    sex_t sex(); ///< @return Parsed sex.
    
  private:
    bool _valid;
    
    std::string _cpr;
    std::string _efternavn;
    std::string _fornavn;
    std::string _conavn;
    std::string _lokalitet;
    std::string _adresse;
    std::string _bynavn;
    std::string _postnr;
    std::string _postby;
    std::string _kommunenr;
    std::string _sikringsgruppe;
    std::string _laegenr;
    std::string _beskyttelse;
    std::string _reserveret;
    std::string _aktion;
    std::string _doedsdato;
    std::string _insert_at;
    std::string _moved_at;
    std::string _num_fields;
  };

};

#endif/*__ARTEFACT_CPR_H__*/
