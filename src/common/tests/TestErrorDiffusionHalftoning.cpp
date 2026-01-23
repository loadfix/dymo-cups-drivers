#include "TestErrorDiffusionHalftoning.h"
#include "TestCommon.h"
#include <ostream>

using namespace std;
using namespace DymoPrinterDriver;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ErrorDiffusionTest);

void
ErrorDiffusionTest::setUp()
{
}


void
ErrorDiffusionTest::tearDown()
{
}


void
ErrorDiffusionTest::testBlack()
{
  // 5 pixels wide, 24bits per pixel
  byte InputLine[] =
    {
      0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    };
  byte OutputData[] = { 0xf8 };

  buffer_t OutputLine;
  CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);
}

void
ErrorDiffusionTest::testWhite()
{
  // 5 pixels wide, 24bits per pixel
  byte InputLine[] =
    {
      0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,
    };
  byte OutputData[] = { 0x00 };

  buffer_t OutputLine;
  CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    OutputLine);
}

void
ErrorDiffusionTest::testRed()
{
  // 5 pixels wide, 24bits per pixel
  byte InputLine[] =
    {
      0xff, 0x00, 0x00,  0xff, 0x00, 0x00,  0xff, 0x00, 0x00,  0xff, 0x00, 0x00,  0xff, 0x00, 0x00,
    };
  byte OutputData1[] = { 0xe8 };
  byte OutputData2[] = { 0xb8 };
  byte OutputData3[] = { 0xd0 };

  buffer_t OutputLine;
  CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData1, OutputData1 + sizeof(OutputData1)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData2, OutputData2 + sizeof(OutputData2)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData3, OutputData3 + sizeof(OutputData3)),
    OutputLine);
}

void
ErrorDiffusionTest::testGreen()
{
  // 5 pixels wide, 24bits per pixel
  byte InputLine[] =
    {
      0x00, 0xff, 0x00,  0x00, 0xff, 0x00,  0x00, 0xff, 0x00,  0x00, 0xff, 0x00,  0x00, 0xff, 0x00
    };
  byte OutputData1[] = { 0x50 };
  byte OutputData2[] = { 0x48 };
  byte OutputData3[] = { 0x50 };

  buffer_t OutputLine;
  CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData1, OutputData1 + sizeof(OutputData1)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData2, OutputData2 + sizeof(OutputData2)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData3, OutputData3 + sizeof(OutputData3)),
    OutputLine);
}

void
ErrorDiffusionTest::testBlue()
{
  // 5 pixels wide, 24bits per pixel
  byte InputLine[] =
    {
      0x00, 0x00, 0xff,  0x00, 0x00, 0xff,  0x00, 0x00, 0xff,  0x00, 0x00, 0xff,  0x00, 0x00, 0xff
    };
  byte OutputData1[] = { 0xf8 };
  byte OutputData2[] = { 0xf8 };
  byte OutputData3[] = { 0xe8 };

  buffer_t OutputLine;
  CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData1, OutputData1 + sizeof(OutputData1)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData2, OutputData2 + sizeof(OutputData2)),
    OutputLine);

  H.ProcessLine(buffer_t(InputLine, InputLine + sizeof(InputLine)), OutputLine);
  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData3, OutputData3 + sizeof(OutputData3)),
    OutputLine);
}
