#ifndef TEST_LABELWRITER_FILTER_H
#define TEST_LABELWRITER_FILTER_H

#include <cppunit/extensions/HelperMacros.h>
#include "MOCK_PrintEnvironment.h"
#include "../LabelWriterDriver.h"
#include "../DummyLanguageMonitor.h"
#include <cups/cups.h>

class LabelWriterFilterTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LabelWriterFilterTest);

  CPPUNIT_TEST(testDensity);
  CPPUNIT_TEST(testQuality);
  CPPUNIT_TEST(testRoll);


  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDensity();
  void testQuality();
  void testRoll();

private:
  ppd_file_t* ppd_;
  MockPrintEnvironment Env_;
  CLabelWriterDriverTwinTurbo* Driver_;
  CDummyLanguageMonitor* LM_;

};

#endif // TEST_LABELWRITER_FILTER_H
