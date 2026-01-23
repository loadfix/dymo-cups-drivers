#ifndef TEST_NLL_HALFTONING_H
#define TEST_NLL_HALFTONING_H

#include <cppunit/extensions/HelperMacros.h>
#include "../NonLinearLaplacianHalftoning.h"

class NonLinearLaplacianTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(NonLinearLaplacianTest);

  CPPUNIT_TEST(testBlock);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testBlock();
};

#endif // TEST_NLL_HALFTONING_H
