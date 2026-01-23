#include "TestNLLHalftoning.h"
#include "TestCommon.h"
#include <ostream>

using namespace std;
using namespace DymoPrinterDriver;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(NonLinearLaplacianTest);

void
NonLinearLaplacianTest::setUp()
{
}


void
NonLinearLaplacianTest::tearDown()
{
}


void
NonLinearLaplacianTest::testBlock()
{
  // 8x10 pixels wide, 24bits per pixel
  byte pInputImage[] =
    {
      255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 0,0,0,       0,0,0,       0,0,0,       255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255,
      255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255,
    };
  byte OutputData[] =
    {
      0x00, 0,
      0x38, 0,
      0x38, 0,
      0x38, 0,
      0x38, 0,
      0x38, 0,
      0x38, 0,
      0x38, 0,
      0x00, 0,
      0x00, 0,
    };

  buffer_t OutputLine;
  CNLLHalftoning H(0, CHalftoneFilter::itRGB, CHalftoneFilter::itBW);

  CHalftoneFilter::image_buffer_t InputImage;
  for (size_t i = 0; i < 10; ++i)
  {
    buffer_t line(pInputImage + i * 8 * 3, pInputImage + (i + 1) * 8 * 3);
    InputImage.push_back(line);
  }

  CHalftoneFilter::image_buffer_t OutputImage;
  H.ProcessImage(InputImage, OutputImage);

  buffer_t Output;
  for (size_t i = 0; i < OutputImage.size(); ++i)
    Output.insert(Output.end(), OutputImage[i].begin(), OutputImage[i].end());

  CPPUNIT_ASSERT_EQUAL(
    buffer_t(OutputData, OutputData + sizeof(OutputData)),
    Output);
}
