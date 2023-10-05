/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <cppunit/extensions/HelperMacros.h>

#include "../../lib/exception.cc"
#include "../../lib/log.cc"
#include "../../lib/uid.cc"
#include "../../lib/entry.cc"
#include "../../lib/utf8.cc"

#include "../postgresql.h"
#include "../postgresql.cc"

class test_postgresql : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(test_postgresql);
	CPPUNIT_TEST(test_connect);
	CPPUNIT_TEST(test_transaction);
	CPPUNIT_TEST(test_post);
	CPPUNIT_TEST(test_get);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp() {}
	void tearDown() {}

  void test_connect() {
		CPPUNIT_ASSERT_NO_THROW(
    PostgreSQL pg("localhost", 5432, "artefact", "artefact", "")
                           );

	}

  void test_transaction() {
    PostgreSQL pg("localhost", 5432, "artefact", "artefact", "");
    PostgreSQLTransaction *t = NULL;
		CPPUNIT_ASSERT_NO_THROW(t = pg.transaction(true));
    delete t;
	}

  void test_post()
  {
    PostgreSQL pg("localhost", 5432, "artefact", "artefact", "");
    PostgreSQLTransaction *t = NULL;
    CPPUNIT_ASSERT_NO_THROW(t = pg.transaction(true));

    Pentominos::Entry entry;
    entry.setPatientID("1011111111");
    Pentominos::Data data;
    CPPUNIT_ASSERT_NO_THROW(t->post(entry, data));
    delete t;
  }

  void test_get()
  {
    /*
    PostgreSQL pg("localhost", 5432, "artefact", "artefact", "");
    PostgreSQLTransaction *t = pg.transaction(true);

    Pentominos::Entry entry;
    std::list< Pentominos::Data > lst;
    CPPUNIT_ASSERT_NO_THROW(lst = t->get("1011111111", "*"));

    CPPUNIT_ASSERT(lst.size() != 0);

    delete t;
    */
  }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(test_postgresql);
