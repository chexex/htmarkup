//-------------------------------------------------------------------------
/// @file   runner.cpp
/// @brief  run cppunit tests
//-------------------------------------------------------------------------

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <iostream>
#include <cstdlib>

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  
  // run all test, default
  if (argc < 2) {
    runner.addTest( registry.makeTest() );
    return runner.run("", false) ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  
  // run specified tests only
  argc--;
  argv++;
  
  for (; argc != 0; argc--, argv++) 
  {
    const char *testName = argv[0];
    std::cout << "=================================================================\n" \
                 "Running " << testName << "\n" \
                 "=================================================================\n";

    runner.addTest( registry.makeTest() );
    bool ok = runner.run(testName, false);
    if (!ok)
      return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
