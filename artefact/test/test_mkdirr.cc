/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
#include <cppunit/extensions/HelperMacros.h>

#include "../../lib/exception.cc"
#include "../../lib/log.cc"

#include "../../lib/mkdirr.h"
#include "../../lib/mkdirr.cc"

#include <stdlib.h>
#include <unistd.h>

// Originally declared in artefact.cc but we cannot include that since main
// resides there...
class test_mkdirr : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(test_mkdirr);
	CPPUNIT_TEST(test1);
	CPPUNIT_TEST_SUITE_END();

public:
  std::string dir;

	void setUp() {
    char templ[] = "/tmp/cppunit_atf_XXXXXX";
    dir = mkdtemp(templ);
    printf("Using basedir: \"%s\"\n", dir.c_str());
  }

	void tearDown() {
    std::string command;
    command = "chmod +rwx -R ";
    command += dir;
    if(system(command.c_str())) {}

    command = "rm -Rf ";
    command += dir;
    if(system(command.c_str())) {}
  }

  void test1() {
    CPPUNIT_ASSERT_NO_THROW(Pentominos::mkdirr(dir+"//a1//b1"));
    
    CPPUNIT_ASSERT_NO_THROW(Pentominos::mkdirr(dir+"/a1/a2"));
    
    CPPUNIT_ASSERT_NO_THROW(Pentominos::mkdirr(dir+"/a1/a2"));
    
    CPPUNIT_ASSERT_NO_THROW(Pentominos::mkdirr(
                            std::string("/")+dir+"/b1/b2//"));
    
    std::string nobrowse0 = dir+"/nobrowse";
    std::string nobrowse = dir+"/nobrowse/nobrowse";
    mkdir(nobrowse0.c_str(), 0777);
    mkdir(nobrowse.c_str(), 0000);
    chmod(nobrowse0.c_str(), 0000);
    CPPUNIT_ASSERT_THROW(Pentominos::mkdirr(nobrowse+"/fisk"),
                         Pentominos::MkdirRDirectoryException);
    chmod(nobrowse0.c_str(), 0777);
    chmod(nobrowse.c_str(), 0777);

    std::string nowrite = dir+"/nowrite";
    mkdir(nowrite.c_str(), 0555);
    CPPUNIT_ASSERT_THROW(Pentominos::mkdirr(nowrite + "/fisk"),
                         Pentominos::MkdirRDirectoryException);
    chmod(nowrite.c_str(), 0777);

    CPPUNIT_ASSERT_EQUAL(Pentominos::stripFilename(dir+"/a1/a2"), dir+"/a1");

    CPPUNIT_ASSERT_EQUAL(Pentominos::stripFilename(dir+"/b1/b2/"),
                         dir+"/b1/b2");

    std::string file3 = dir+"/artefact_data/data/"
      "1945/6/27/2706450079//1336645064-tonoref2";
    CPPUNIT_ASSERT_NO_THROW(Pentominos::mkdirr(
                            Pentominos::stripFilename(file3)));
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(test_mkdirr);
