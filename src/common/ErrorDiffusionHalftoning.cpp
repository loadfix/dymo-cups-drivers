#include "ErrorDiffusionHalftoning.h"
#include <algorithm>

namespace DymoPrinterDriver
{


ErrorDiffusionHalftoning::ErrorDiffusionHalftoning(image_t InputImageType, image_t OutputImageType, bool UsePrinterColorSpace):
  HalftoneFilter(InputImageType, OutputImageType), ImageWidth_(0), Errors_(), GrayLine_(), UsePrinterColorSpace_(UsePrinterColorSpace)
{
  if (GetOutputImageType() != itBW)
    throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
}

ErrorDiffusionHalftoning::~CErrorDiffusionHalftoning()
{
}


bool
ErrorDiffusionHalftoning::IsProcessLineSupported()
{
  return true;
}

void
ErrorDiffusionHalftoning::ProcessLine(
  const buffer_t& InputLine, buffer_t& OutputLine)
{
  int       pixelValue  = 0;
  int       error       = 0;
  size_t    i           = 0;

  // set image  width
  if (!ImageWidth_)
    ImageWidth_ = CalcImageWidth(InputLine);

  // check buffer size
  OutputLine.resize(CalcOutputBufferSize(ImageWidth_));
  std::fill(OutputLine.begin(), OutputLine.end(), byte(0));

  // initialize halftone errors array and line buffer
  if (Errors_.size() == 0)
    Errors_.resize(ImageWidth_, 0);

  if (GrayLine_.size() == 0)
    GrayLine_.resize(ImageWidth_, 0);

  // convert from RGB to GrayScale
  for (i = 0; i < ImageWidth_; ++i)
  {
    byte R, G, B;
    ExtractRGB(InputLine, i, R, G, B);
    GrayLine_[i] = RGBToGrayScale(R, G, B);
  }

  // apply errors from prev line
  for (i = 0; i < ImageWidth_; ++i)
  {
    //fprintf(stderr, "%i ", Errors_[i]);
    // only if not black and white
    if ((GrayLine_[i] != 0) && (GrayLine_[i] != 255))
    {
      if (Errors_[i] + GrayLine_[i] >= 255)
        GrayLine_[i] = 255;
      else
        if (Errors_[i] + GrayLine_[i] <= 0)
          GrayLine_[i] = 0;
        else
          GrayLine_[i] += Errors_[i];
    }

    Errors_[i] = 0;
  }


  // compute output pixels and new errors
  for (i = 0; i < ImageWidth_; ++i)
  {
    pixelValue = GrayLine_[i] >= 128;
    error = GrayLine_[i] - pixelValue * 255;

    if (UsePrinterColorSpace_)
      SetPixelBW(OutputLine, i, !pixelValue);
    else
      SetPixelBW(OutputLine, i, pixelValue);


    // disribute error
    if (i > 0)
      Errors_[i - 1] += (error * 3) / 16;

    Errors_[i] += (error * 5) / 16;

    if (i < ImageWidth_ - 1)
    {
      Errors_[i + 1] += (error * 1) / 16;

      if ((GrayLine_[i + 1] != 0) && (GrayLine_[i + 1] != 255))
      {
        error = (error * 7) / 16;

        if (error + GrayLine_[i + 1] >= 255)
          GrayLine_[i + 1] = 255;
        else
          if (error + GrayLine_[i + 1] <= 0)
            GrayLine_[i + 1] = 0;
          else
            GrayLine_[i + 1] += error;
      }
    }

  } // for all pixels
}

void
ErrorDiffusionHalftoning::ProcessImage(
  const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();

  buffer_t InputLine;
  size_t   BufferSize = CalcBufferSize(ImageWidth);
  InputLine.resize(BufferSize, 0);

  for (size_t i = 0; i < ImageHeight; ++i)
  {
    InputLine.assign(
      (byte*)ImageData + LineDelta*i,
      (byte*)ImageData + LineDelta*i + BufferSize);

    OutputImage.push_back(buffer_t());
    ProcessLine(InputLine, OutputImage[OutputImage.size() - 1]);
  }
}

void
ErrorDiffusionHalftoning::ProcessImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();

  buffer_t OutputLine;

  for (std::vector<buffer_t>::const_iterator i = InputImage.begin(); i < InputImage.end(); ++i)
  {
    ProcessLine(*i, OutputLine);
    OutputImage.push_back(OutputLine);
  }
}


} // namespace
