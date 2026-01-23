#include "Halftoning.h"
#include <algorithm>
#include <assert.h>

namespace DymoPrinterDriver
{

HalftoneFilter::HalftoneFilter(image_t input_image_type, image_t output_image_type):
  InputImageType_(input_image_type), OutputImageType_(output_image_type)
{
}

HalftoneFilter::~HalftoneFilter()
{
}

HalftoneFilter::image_t
HalftoneFilter::getInputImageType()
{
  return InputImageType_;
}

HalftoneFilter::image_t
HalftoneFilter::getOutputImageType()
{
  return OutputImageType_;
}

byte
HalftoneFilter::convertRGBToGrayScale(byte r, byte g, byte b)
{
  // white should remain white
  if ((r == 255) && (g == 255) && (b == 255))
    return 255;
  else if ((r == 0) && (g == 0) && (b == 0))
    return 0;
  else
  {
    int r_val = 0 + ((int(r) * 299) / 1000) + ((int(g) * 587) / 1000) + ((int(b) * 114) / 1000);
    if (r_val > 255)
      return 255;
    return byte(r_val);
  }
}

// set pixel pixelNo to
// pixelValue (0 - white, 1 - black)
void
HalftoneFilter::setPixelBW(buffer_t& buf, int pixel_no, int pixel_value)
{
  if (pixel_value)
    buf[pixel_no / 8] |= (1 << (7 - pixel_no % 8));
  else
    buf[pixel_no / 8] &= ~(1 << (7 - pixel_no % 8));
}

void
HalftoneFilter::extractRGB(const buffer_t& input_line, int pixel_no, byte& r, byte& g, byte& b)
{
  switch (InputImageType_)
  {
    case itXRGB:
      r = input_line[4*pixel_no + 1];
      g = input_line[4*pixel_no + 2];
      b = input_line[4*pixel_no + 3];
      break;
    case itRGB:
      r = input_line[3*pixel_no + 0];
      g = input_line[3*pixel_no + 1];
      b = input_line[3*pixel_no + 2];
      break;
    default:
      assert(0);
  }
}

size_t
HalftoneFilter::calcImageWidth(const buffer_t& input_line)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return input_line.size() / 4;
    case itRGB:
      return input_line.size() / 3;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}


size_t
HalftoneFilter::calcBufferSize(size_t image_width)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return image_width * 4;
    case itRGB:
      return image_width * 3;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

size_t
HalftoneFilter::calcOutputBufferSize(size_t image_width)
{
  switch (OutputImageType_)
  {
    case itBW:
      if (image_width % 8 == 0)
        return image_width / 8;
      else
        return image_width / 8 + 1;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

int
HalftoneFilter::extractRGB(const buffer_t& input_line, int pixel_no)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return
        (int(input_line[4*pixel_no + 1]) << 16)
        | (int(input_line[4*pixel_no + 2]) << 8)
        | (input_line[4*pixel_no + 3] );
    case itRGB:
      return
        (int(input_line[3*pixel_no + 0]) << 16)
        | (int(input_line[3*pixel_no + 1]) << 8)
        | (input_line[3*pixel_no + 2] );
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

/////////////////////////////////////////////////////////////////////////
// EHalftoneError
/////////////////////////////////////////////////////////////////////////

EHalftoneError::EHalftoneError(error_t error_code): ErrorCode_(error_code)
{
}

EHalftoneError::error_t
EHalftoneError::getErrorCode()
{
  return ErrorCode_;
}


} // namespace
