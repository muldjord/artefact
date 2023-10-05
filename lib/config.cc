/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *       config.cc
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
#include "config.h"
#include <stdlib.h>

// The global config object
Pentominos::Configs Pentominos::config;

Pentominos::Variant::Variant(std::string val)
{
  str_val = val;
  int_val = atoi(val.c_str());
  double_val = atof(val.c_str());
  bool_val = (val != "");
  //  Configs configs_val;
  _type = TYPE_STRING;
}

Pentominos::Variant::Variant(int val)
{
  char buf[40]; // Big enough for a 128 bit integer.
  sprintf(buf, "%d", val);
  str_val = buf;
  int_val = val;
  double_val = val;
  bool_val = (val != 0);
  //  Configs configs_val;
  _type = TYPE_INTEGER;
}

Pentominos::Variant::Variant(double val)
{
  char buf[128]; // doubles tend to get pretty big!
  sprintf(buf, "%f", val);
  str_val = buf;
  int_val = (int)val;
  double_val = val;
  bool_val = (val <= 0 && val >= 0);
  //  Configs configs_val;
  _type = TYPE_NUMBER;
}

Pentominos::Variant::Variant(bool val)
{
  str_val = val?"true":"false";
  int_val = (int)val;
  double_val = (double)val;
  bool_val = val;
  //  Configs configs_val;
  _type = TYPE_BOOLEAN;
}

Pentominos::Variant::Variant(struct in_addr val)
{
  /*
  str_val = val?"true":"false";
  int_val = (int)val;
  double_val = (double)val;
  bool_val = val;
  //  Configs configs_val;
  */
  in_addr_val = val;

  _type = TYPE_IP;
}

Pentominos::Variant::Variant(Configs val)
{
  str_val = "block";
  int_val = 0;
  double_val = 0.0;
  bool_val = false;
  configs_val = val;
  _type = TYPE_BLOCK;
}

Pentominos::type_t Pentominos::Variant::type()
{
  return _type;
}

Pentominos::Variant::operator std::string() const
{
  return str_val;
}

Pentominos::Variant::operator int() const
{
  return int_val;
}

Pentominos::Variant::operator double() const
{
  return double_val;
}

Pentominos::Variant::operator bool() const
{
  return bool_val;
}

Pentominos::Variant::operator struct in_addr() const
{
  return in_addr_val;
}

Pentominos::Variant::operator Configs() const
{
  return configs_val;
}

Pentominos::Configs::operator std::map<std::string, std::string>() const
{
  std::map<std::string, std::string> strmap;
  Configs::const_iterator i = begin();
  while(i != end()) {
    std::string name = i->first;
    VariantList vl = i->second;
    std::string value = vl;
    strmap[name] = value;
    i++;
  }
  
  return strmap;
}


#ifdef TEST_CONFIG

void print(Pentominos::Configs &c, std::string indent = "")
{
  Pentominos::Configs::iterator i = c.begin();
  while(i != c.end()) {
    printf("%s%s = ", indent.c_str(), (*i).first.c_str());

    Pentominos::VariantList::iterator j = (*i).second.begin();
    while(j != (*i).second.end()) {
			if((*j).type() == Pentominos::TYPE_BLOCK) {
				Pentominos::Configs cfgs = (*j);
				printf("{\n");
				print(cfgs, indent + "\t");
				printf("%s}\n", indent.c_str());
			} else {
				std::string s = (*j);
				printf("%s ", s.c_str());
			}
      j++;
    }
    printf("\n");

    i++;
  }
}

int main()
{

  // Test of the Variant class
  {
    int a = 100;

    Pentominos::Variant variant(a);

    int b = variant;
    std::string c = variant;
    double e = variant;

    printf("a = %d\nb = %d\nc = %s\ne = %f\n", a, b, c.c_str(), e);
  }

  {
    std::string a = "100.42";

    Pentominos::Variant variant(a);

    int b = variant;
    std::string c = variant;
    double e = variant;

    printf("a = %s\nb = %d\nc = %s\ne = %f\n", a.c_str(), b, c.c_str(), e);
  }

  // Test of the Config class
  {
    std::string world = "world";
    std::string fortune = "fortune";
    Pentominos::Variant var1(world);
    Pentominos::Variant var2(fortune);
    Pentominos::config["hello"].push_back(var1);
    Pentominos::config["hello"].push_back(var2);

    // Read out a list of values
    Pentominos::VariantList::iterator i = Pentominos::config["hello"].begin();
    printf("hello = ");
    while(i != Pentominos::config["hello"].end()) {
      std::string hello = *i;
      printf("%s, ", hello.c_str());
      i++;
    }
    printf("\n");

    // Test auto index 0 when no index is supplied.
    Pentominos::Variant meaning(42);
    Pentominos::config["themeaning"] = meaning;

    std::string themeaning = Pentominos::config["themeaning"];
    printf("%s = %s\n", "themeaning", themeaning.c_str());

    // Test blocks
    Pentominos::Configs cfgs;
    cfgs["test1"].push_back(meaning);
    cfgs["test2"].push_back(world);
    Pentominos::config["block"].push_back(cfgs);

    Pentominos::Configs c = Pentominos::config["block"];
    std::string s1 = c["test1"];
    std::string s2 = c["test2"];
    printf("test1 = %s\n", s1.c_str());
    printf("test2 = %s\n", s2.c_str());

    std::map<std::string, std::string> mymap = c;
    for(std::map<std::string, std::string>::iterator i = mymap.begin(); i != mymap.end(); i++) {
      printf("MAP: %s => %s\n", i->first.c_str(), i->second.c_str());
    }
  }

  Pentominos::parse_config((char*)"../etc/artefact.conf");
  print(Pentominos::config);

  return 0;
}

#if 0

void dosomething(Configs &block)
{
  int blah = configs["blah"];
}

void example()
{
  Configs block = config["myblock"];
  int dims = block["dims"];

  Configs::iterator i = config.begin();
  while(i != config.end()) {
    std::string name = (*i).first; // navnet
    Variant val = (*i).second; // value
    if(val.type == TYPE_BLOCK) dosomething(val);
    i++;
  }

  double comp = config["compression"];

  // a = 1,2
  int a0 = config["a"][0];
  int a1 = config["a"][1];

  int compression = config["compression"];
}

#endif/*0*/

#endif/*TEST_CONFIG*/
