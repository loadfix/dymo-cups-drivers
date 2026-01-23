#include "ErrorDiffusionHalftoning.h"
#include <algorithm>

namespace DymoPrinterDriver
{


ErrorDiffusionHalftoning::ErrorDiffusionHalftoning(image_t input_image_type, image_t output_image_type, bool use_printer_color_space):
  HalftoneFilter(input_image_type, output_image_type), imageWidth(0), error(), grayLine(), usePrinterColorSpace(use_printer_color_space)
{
  if (getOutputImageType() != itBW)
    throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
}

ErrorDiffusionHalftoning::~ErrorDiffusionHalftoning()
{
}


bool
ErrorDiffusionHalftoning::isProcessLineSupported()
{
  return true;
}

void
ErrorDiffusionHalftoning::processLine(
  const buffer_t& InputLine, buffer_t& OutputLine)
{
  int       pixelValue  = 0;
  int       error       = 0;
  size_t    i           = 0;

  // set image  width
  if (!imageWidth)
    imageWidth = calcImageWidth(InputLine);

  // check buffer size
  OutputLine.resize(calcOutputBufferSize(imageWidth));
  std::fill(OutputLine.begin(), OutputLine.end(), byte(0));

  // initialize halftone errors array and line buffer
  if (error.size() == 0)
    error.resize(imageWidth, 0);

  if (grayLine.size() == 0)
    grayLine.resize(imageWidth, 0);

  // convert from RGB to GrayScale
  for (i = 0; i < imageWidth; ++i)
  {
    byte R, G, B;
    extractRGB(InputLine, i, R, G, B);
    grayLine[i] = convertRGBToGrayScale(R, G, B);
  }

  // apply errors from prev line
  for (i = 0; i < imageWidth; ++i)
  {
    //fprintf(stderr, "%i ", error[i]);
    // only if not black and white
    if ((grayLine[i] != 0) && (grayLine[i] != 255))
    {
      if (error[i] + grayLine[i] >= 255)
        grayLine[i] = 255;
      else
        if (error[i] + grayLine[i] <= 0)
          grayLine[i] = 0;
        else
          grayLine[i] += error[i];
    }

    error[i] = 0;
  }


  // compute output pixels and new errors
  for (i = 0; i < imageWidth; ++i)
  {
    pixelValue = grayLine[i] >= 128;
    int pixelError = grayLine[i] - pixelValue * 255;

    if (usePrinterColorSpace)
      setPixelBW(OutputLine, i, !pixelValue);
    else
      setPixelBW(OutputLine, i, pixelValue);


    // disribute error
    if (i > 0)
      error[i - 1] += (pixelError * 3) / 16;

    error[i] += (pixelError * 5) / 16;

    if (i < imageWidth - 1)
    {
      error[i + 1] += (pixelError * 1) / 16;

      if ((grayLine[i + 1] != 0) && (grayLine[i + 1] != 255))
      {
        int nextPixelError = (pixelError * 7) / 16;

        if (nextPixelError + grayLine[i + 1] >= 255)
          grayLine[i + 1] = 255;
        else
          if (nextPixelError + grayLine[i + 1] <= 0)
            grayLine[i + 1] = 0;
          else
            grayLine[i + 1] += nextPixelError;
      }
    }

  } // for all pixels
}

void
ErrorDiffusionHalftoning::processImage(
  const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();

  buffer_t InputLine;
  size_t   BufferSize = calcBufferSize(ImageWidth);
  InputLine.resize(BufferSize, 0);

  for (size_t i = 0; i < ImageHeight; ++i)
  {
    InputLine.assign(
      (byte*)ImageData + LineDelta*i,
      (byte*)ImageData + LineDelta*i + BufferSize);

    OutputImage.push_back(buffer_t());
    processLine(InputLine, OutputImage[OutputImage.size() - 1]);
  }
}

void
ErrorDiffusionHalftoning::processImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();

  buffer_t OutputLine;

  for (std::vector<buffer_t>::const_iterator i = InputImage.begin(); i < InputImage.end(); ++i)
  {
    processLine(*i, OutputLine);
    OutputImage.push_back(OutputLine);
  }
}


} // namespace
