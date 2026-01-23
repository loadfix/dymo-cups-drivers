#ifndef TEST_ERROR_DIFFUSION_HALFTONING_H
#define TEST_ERROR_DIFFUSION_HALFTONING_H

#include <cppunit/extensions/HelperMacros.h>
#include "../ErrorDiffusionHalftoning.h"

class ErrorDiffusionTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ErrorDiffusionTest);

  CPPUNIT_TEST(testBlack);
  CPPUNIT_TEST(testWhite);
  CPPUNIT_TEST(testRed);
  CPPUNIT_TEST(testGreen);
  CPPUNIT_TEST(testBlue);


  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testBlack();
  void testWhite();
  void testRed();
  void testGreen();
  void testBlue();
};

#endif // TEST_ERROR_DIFFUSION_HALFTONING_H
