#ifndef TEST_LABELWRITER_DRIVER_H
#define TEST_LABELWRITER_DRIVER_H

#include <cppunit/extensions/HelperMacros.h>
#include "../LabelWriterDriver.h"

class LabelWriterDriverTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LabelWriterDriverTest);

  CPPUNIT_TEST(testPrologEpilogCommand);
  CPPUNIT_TEST(testCompression);
  CPPUNIT_TEST(testEmptyLines);
  CPPUNIT_TEST(testDotTab);
  CPPUNIT_TEST(testContinuousPaper);
  CPPUNIT_TEST(test400EndPageEndDoc);
  CPPUNIT_TEST(testTwinTurboRoll);


  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testPrologEpilogCommand();
  void testCompression();
  void testEmptyLines();
  void testDotTab();
  void testContinuousPaper();

  void test400EndPageEndDoc();
  void testTwinTurboRoll();

};

#endif // TEST_LABELWRITER_DRIVER_H
