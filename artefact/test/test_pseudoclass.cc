/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <cppunit/extensions/HelperMacros.h>

#include "../../lib/debug.cc"
#include "../../lib/exception.cc"
#include "../../lib/log.cc"
#include "../../lib/uid.cc"
#include "../../lib/lua.cc"
#include "../../lib/luautil.cc"
#include "../../lib/utf8.cc"
#include "../../lib/tcpsocket.cc"
#include "../../lib/tostring.cc"
#include "../../lib/config.cc"
#include "../../lib/entry.cc"

#include "../postgresql.cc"
#include "../luatcpsocket.cc"
#include "../luaexpat.cc"
#include "../luapostgresql.cc"
#include "../luadatainterpreter.cc"

#include "../queryhandler.h"
#include "../queryhandler.cc"

// Originally declared in artefact.cc but we cannot include that since main
// resides there...
std::string lua_basedir;
std::vector< std::string > static_classes;

class test_pseudoclass : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(test_pseudoclass);
	CPPUNIT_TEST(test_latest_pretend);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

  void test_latest_pretend() {
    std::vector<std::string> cls;
    cls.push_back("iolmaster500_keratometer");
    cls.push_back("iolmaster500_iol");
    pseudo_classes["iolmaster500"] = cls;

    
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(test_pseudoclass);

