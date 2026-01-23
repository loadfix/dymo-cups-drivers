#include "Halftoning.h"
#include <algorithm>
#include <assert.h>

namespace DymoPrinterDriver
{

CHalftoneFilter::CHalftoneFilter(image_t InputImageType, image_t OutputImageType):
  InputImageType_(InputImageType), OutputImageType_(OutputImageType)
{
}

CHalftoneFilter::~CHalftoneFilter()
{
}

CHalftoneFilter::image_t
CHalftoneFilter::GetInputImageType()
{
  return InputImageType_;
}

CHalftoneFilter::image_t
CHalftoneFilter::GetOutputImageType()
{
  return OutputImageType_;
}

byte
CHalftoneFilter::RGBToGrayScale(byte R, byte G, byte B)
{
  // white should remain white
  if ((R == 255) && (G == 255) && (B == 255))
    return 255;
  else if ((R == 0) && (G == 0) && (B == 0))
    return 0;
  else
  {
    int r = 0 + ((int(R) * 299) / 1000) + ((int(G) * 587) / 1000) + ((int(B) * 114) / 1000);
    if (r > 255)
      return 255;
    return byte(r);
  }
}

// set pixel pixelNo to
// pixelValue (0 - white, 1 - black)
void
CHalftoneFilter::SetPixelBW(buffer_t& buf, int pixelNo, int pixelValue)
{
  if (pixelValue)
    buf[pixelNo / 8] |= (1 << (7 - pixelNo % 8));
  else
    buf[pixelNo / 8] &= ~(1 << (7 - pixelNo % 8));
}

void
CHalftoneFilter::ExtractRGB(const buffer_t& InputLine, int PixelNo, byte& R, byte& G, byte& B)
{
  switch (InputImageType_)
  {
    case itXRGB:
      R = InputLine[4*PixelNo + 1];
      G = InputLine[4*PixelNo + 2];
      B = InputLine[4*PixelNo + 3];
      break;
    case itRGB:
      R = InputLine[3*PixelNo + 0];
      G = InputLine[3*PixelNo + 1];
      B = InputLine[3*PixelNo + 2];
      break;
    default:
      assert(0);
  }
}

size_t
CHalftoneFilter::CalcImageWidth(const buffer_t& InputLine)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return InputLine.size() / 4;
    case itRGB:
      return InputLine.size() / 3;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}


size_t
CHalftoneFilter::CalcBufferSize(size_t ImageWidth)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return ImageWidth * 4;
    case itRGB:
      return ImageWidth * 3;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

size_t
CHalftoneFilter::CalcOutputBufferSize(size_t ImageWidth)
{
  switch (OutputImageType_)
  {
    case itBW:
      if (ImageWidth % 8 == 0)
        return ImageWidth / 8;
      else
        return ImageWidth / 8 + 1;
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

int
CHalftoneFilter::ExtractRGB(const buffer_t& InputLine, int PixelNo)
{
  switch (InputImageType_)
  {
    case itXRGB:
      return
        (int(InputLine[4*PixelNo + 1]) << 16)
        | (int(InputLine[4*PixelNo + 2]) << 8)
        | (InputLine[4*PixelNo + 3] );
    case itRGB:
      return
        (int(InputLine[3*PixelNo + 0]) << 16)
        | (int(InputLine[3*PixelNo + 1]) << 8)
        | (InputLine[3*PixelNo + 2] );
    default:
      assert(0);
  }

  return 0; // for MSVC compiler
}

/////////////////////////////////////////////////////////////////////////
// EHalftoneError
/////////////////////////////////////////////////////////////////////////

EHalftoneError::EHalftoneError(error_t ErrorCode): ErrorCode_(ErrorCode)
{
}

EHalftoneError::error_t
EHalftoneError::GetErrorCode()
{
  return ErrorCode_;
}


} // namespace
