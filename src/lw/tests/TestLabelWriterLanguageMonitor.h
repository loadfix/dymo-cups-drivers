#ifndef TEST_LABELWRITER_LANGUAGE_MONITOR_H
#define TEST_LABELWRITER_LANGUAGE_MONITOR_H

#include <cppunit/extensions/HelperMacros.h>
#include "../LabelWriterDriver.h"

class LabelWriterLMTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LabelWriterLMTest);

  CPPUNIT_TEST(testGoodJob);
  CPPUNIT_TEST(testPaperOut);
  CPPUNIT_TEST(testPaperOutAfterReprint);
  CPPUNIT_TEST(testRollSynchronize);
  CPPUNIT_TEST(testResetPrinter);
  CPPUNIT_TEST(testPollForPaperIn);
  CPPUNIT_TEST(testOneGoodPageOneNot);
  CPPUNIT_TEST(testStatusReadFailed);
  CPPUNIT_TEST(testContinuousPaper);
  CPPUNIT_TEST(testRollChangedBit);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  // test a job when there is no any problems
  void testGoodJob();
  void testPaperOut();
  void testPaperOutAfterReprint();    // reprint a page twice
  void testRollSynchronize();
  void testResetPrinter();
  void testPollForPaperIn();          // test waiting for inserting paper
  void testOneGoodPageOneNot();
  void testStatusReadFailed();
  void testContinuousPaper();
  void testRollChangedBit();

};

#endif // TEST_LABELWRITER_LANGUAGE_MONITOR_H
