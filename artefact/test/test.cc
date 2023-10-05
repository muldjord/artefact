/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

//#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <fstream>

int main(int argc, char* argv[])
{
  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  // runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

  std::ofstream myfile;
  myfile.open("result_"OUTPUT".xml");
  runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), myfile));

  // Run the tests.
  bool wasSucessful = runner.run();

  myfile.close();

  // Return error code 1 if the one of test failed.
  return wasSucessful ? 0 : 1;
}
