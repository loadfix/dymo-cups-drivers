#ifndef TEST_LABELMANAGER_DRIVER_H
#define TEST_LABELMANAGER_DRIVER_H

#include <cppunit/extensions/HelperMacros.h>
#include "../LabelManagerDriver.h"

class LabelManagerDriverTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LabelManagerDriverTest);

  //CPPUNIT_TEST(testPrologEpilogCommand);
  CPPUNIT_TEST(testEmptyLines);
  CPPUNIT_TEST(testEmptyLinesAtEnd);
  CPPUNIT_TEST(testDotTab);
  CPPUNIT_TEST(testCutAndChainMarks);
  CPPUNIT_TEST(testAlignment);
  CPPUNIT_TEST(testMinPageLength);
  CPPUNIT_TEST(testReverseData);
  CPPUNIT_TEST(testContinuousPaper);
  CPPUNIT_TEST(testAutoPaper);
  CPPUNIT_TEST(testShiftData);
  CPPUNIT_TEST(testMaxPrintableWidth);
  CPPUNIT_TEST(testTapeAlignmentOffset);


  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  //void testPrologEpilogCommand();
  void testEmptyLines();
  void testEmptyLinesAtEnd();
  void testDotTab();
  void testCutAndChainMarks();
  void testAlignment();
  void testMinPageLength();
  void testReverseData();
  void testContinuousPaper();
  void testAutoPaper();
  void testShiftData();
  void testMaxPrintableWidth();
  void testTapeAlignmentOffset();

};

#endif // TEST_LABELMANAGER_DRIVER_H
