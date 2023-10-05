/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *       config.h
 *
 * Mon Oct 22 11:50:14 CEST 2007
 * Copyright 2007 Bent Bisballe Nyeng
 * deva@aasimon.org
 ****************************************************************************/

/*
 * Config is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Configis distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Config; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <vector>
#include <map>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

// The generated parser
#include "y.tab.h"

#include "exception.h"

namespace Pentominos {

  /**
   * This exception is thrown by Config when reload fails.
   */
  class ConfigReloadException: public Pentominos::Exception {
  public:
    ConfigReloadException(std::string reason) : 
      Pentominos::Exception(reason) {}
  };

  /**
   * This exception is thrown by Config when an unknown config is
   * being requested.
   */
  class ConfigNoSuchConfigException: public Pentominos::Exception {
  public:
    ConfigNoSuchConfigException() : 
      Pentominos::Exception("Lookup of unknown config entry.", LEVEL_WARNING) {}
  };

  typedef enum {
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_IP,
    TYPE_BLOCK
  } type_t;
  
  class VariantList;
  class Configs : public std::map< std::string, VariantList >
  {
  public:
    operator std::map<std::string, std::string>() const;
  };
  
  class Variant {
  public:
    Variant() {} // Empty constructor, for internal use only.
    Variant(std::string val);
    Variant(int val);
    Variant(double val);
    Variant(bool val);
    Variant(struct in_addr val);
    Variant(Configs val);
    
    type_t type();
    
    operator std::string() const;
    operator int() const;
    operator double() const;
    operator bool() const;
    operator struct in_addr() const;
    operator Configs() const;

  private:
    type_t _type;
    std::string str_val;
    int int_val;
    double double_val;
    bool bool_val;
    struct in_addr in_addr_val;
    Configs configs_val;
  };
  
  class VariantList : public std::vector< Variant > {
  public:
    // Typecasts
    operator std::string() const
    {
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }
    
    operator int() const
    {
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }
    
    operator double() const
    {
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }
    
    operator bool() const
    {
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }
    
    operator struct in_addr() const
    { 
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }
    
    operator Configs() const
    {
      if(!size()) throw ConfigNoSuchConfigException();
      std::vector< Variant > v = *this; return v[this->size()-1];
    }

    // Assignments
    void operator=(std::string val)
    {
      this->push_back(Variant(val));
    }
    
    void operator=(int val)
    {
      this->push_back(Variant(val));
    }
    
    void operator=(double val)
    {
      this->push_back(Variant(val));
    }
    
    void operator=(bool val)
    {
      this->push_back(Variant(val));
    }
    
    void operator=(struct in_addr val)
    {
      this->push_back(Variant(val));
    }

    void operator=(Configs val)
    {
      this->push_back(Variant(val));
    }
    
    void operator=(Variant val)
    {
      this->push_back(val);
    }
    
  };
  
  extern Configs config;

  // The parser function. It is implemented in config.y
  void parse_config(std::string filename);
};

#endif/*__CONFIG_H__*/
