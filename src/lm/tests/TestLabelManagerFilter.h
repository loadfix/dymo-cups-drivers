#ifndef TEST_LABELMANAGER_FILTER_H
#define TEST_LABELMANAGER_FILTER_H

#include <cppunit/extensions/HelperMacros.h>
#include "MOCK_PrintEnvironment.h"
#include "../LabelManagerDriver.h"
#include "DummyLanguageMonitor.h"
#include <cups/cups.h>

class LabelManagerFilterTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LabelManagerFilterTest);

  CPPUNIT_TEST(testAlignment);
  CPPUNIT_TEST(testCutOptions);
  CPPUNIT_TEST(testContinuousPaper);
  CPPUNIT_TEST(testTapeColor);
  CPPUNIT_TEST(testDeviceSettings);


  CPPUNIT_TEST_SUITE_END();

public:
  LabelManagerFilterTest();

  void setUp();
  void tearDown();

  void testAlignment();
  void testCutOptions();
  void testContinuousPaper();
  void testTapeColor();
  void testDeviceSettings();

private:
  typedef std::vector<std::string> ppd_names_t;
  typedef std::vector<std::string>::iterator ppd_names_it;

  typedef std::map<string, ppd_file_t*> ppds_t;
  typedef std::map<string, ppd_file_t*>::iterator ppds_it;

  std::vector<std::string> PPDNames_;
  ppds_t ppds_;
  MockPrintEnvironment Env_;
  CLabelManagerDriver* Driver_;
  CDummyLanguageMonitor* LM_;

};

#endif // TEST_LABELMANAGER_FILTER_H
