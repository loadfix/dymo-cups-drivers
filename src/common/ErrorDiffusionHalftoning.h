#ifndef ERROR_DIFFUSION_HALFTONING_H
#define ERROR_DIFFUSION_HALFTONING_H

#include "Halftoning.h"

namespace DymoPrinterDriver
{

class ErrorDiffusionHalftoning: public HalftoneFilter
{
public:
   ErrorDiffusionHalftoning(image_t input_image_type, image_t output_image_type, bool use_printer_color_space = true) : HalftoneFilter(input_image_type, output_image_type), imageWidth(0), error(), grayLine(), usePrinterColorSpace(use_printer_color_space)
   {
      if(getOutputImageType() != itBW)
         throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
   }

   virtual ~ErrorDiffusionHalftoning() {}

   virtual bool isProcessLineSupported() { return true; }

   virtual void processLine(const buffer_t& InputLine, buffer_t& OutputLine)
   {
      int pixelValue = 0;
      size_t i = 0;

      // Set image  width
      if(!imageWidth)
         imageWidth = calcImageWidth(InputLine);

      // Check buffer size
      OutputLine.resize(calcOutputBufferSize(imageWidth));
      std::fill(OutputLine.begin(), OutputLine.end(), byte(0));

      // Initialize halftone errors array and line buffer
      if(error.size() == 0)
         error.resize(imageWidth, 0);

      if(grayLine.size() == 0)
         grayLine.resize(imageWidth, 0);

      // Convert from RGB to GrayScale
      for(i = 0; i < imageWidth; ++i)
      {
         byte R, G, B;
         extractRGB(InputLine, i, R, G, B);
         grayLine[i] = convertRGBToGrayScale(R, G, B);
      }

      // Apply errors from prev line
      for(i = 0; i < imageWidth; ++i)
      {
         // Only if not black and white
         if((grayLine[i] != 0) && (grayLine[i] != 255))
         {
            if(error[i] + grayLine[i] >= 255)
               grayLine[i] = 255;
            else
               if(error[i] + grayLine[i] <= 0)
                  grayLine[i] = 0;
               else
                  grayLine[i] += error[i];
         }

         error[i] = 0;
      }


      // Compute output pixels and new errors
      for(i = 0; i < imageWidth; ++i)
      {
         pixelValue = grayLine[i] >= 128;
         int pixelError = grayLine[i] - pixelValue * 255;

         if(usePrinterColorSpace)
            setPixelBW(OutputLine, i, !pixelValue);
         else
            setPixelBW(OutputLine, i, pixelValue);

         // Disribute error
         if(i > 0)
            error[i - 1] += (pixelError * 3) / 16;

         error[i] += (pixelError * 5) / 16;

         if(i < imageWidth - 1)
         {
            error[i + 1] += (pixelError * 1) / 16;

            if((grayLine[i + 1] != 0) && (grayLine[i + 1] != 255))
            {
               int nextPixelError = (pixelError * 7) / 16;

               if(nextPixelError + grayLine[i + 1] >= 255)
                  grayLine[i + 1] = 255;
               else
                  if(nextPixelError + grayLine[i + 1] <= 0)
                     grayLine[i + 1] = 0;
                  else
                     grayLine[i + 1] += nextPixelError;
            }
         }
      } // for all pixels
   }

   virtual void processImage(const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
   {
      OutputImage.clear();

      buffer_t InputLine;
      size_t BufferSize = calcBufferSize(ImageWidth);
      InputLine.resize(BufferSize, 0);

      for(size_t i = 0; i < ImageHeight; ++i)
      {
         InputLine.assign((byte*)ImageData + LineDelta * i, (byte*)ImageData + LineDelta * i + BufferSize);
         OutputImage.push_back(buffer_t());
         processLine(InputLine, OutputImage[OutputImage.size() - 1]);
      }
   }

   virtual void processImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
   {
      OutputImage.clear();

      buffer_t OutputLine;

      for(std::vector<buffer_t>::const_iterator i = InputImage.begin(); i < InputImage.end(); ++i)
      {
         processLine(*i, OutputLine);
         OutputImage.push_back(OutputLine);
      }
   }

protected:
   size_t getImageWidth() { return imageWidth; }

private:
   size_t imageWidth;         // image width in pixels
   std::vector<int> error;    // error buffer
   std::vector<int> grayLine; // current line in gray scale color
   bool usePrinterColorSpace; // if true then use 1 as black, 0 - as white; otherwise 0 is black 1 - white
};

};

#endif // ERROR_DIFFUSION_HALFTONING_H
