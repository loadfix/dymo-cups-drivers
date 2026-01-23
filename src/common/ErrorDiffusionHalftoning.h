#ifndef ERROR_DIFFUSION_HALFTONING_H
#define ERROR_DIFFUSION_HALFTONING_H

#include "Halftoning.h"

namespace DymoPrinterDriver
{

class CErrorDiffusionHalftoning: public CHalftoneFilter
{
public:
   CErrorDiffusionHalftoning(image_t InputImageType, image_t OutputImageType, bool UsePrinterColorSpace = true) : CHalftoneFilter(InputImageType, OutputImageType), _imageWidth(0), _error(), _grayLine(), _usePrinterColorSpace(UsePrinterColorSpace)
   {
      if(GetOutputImageType() != itBW)
         throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
   }

   virtual ~CErrorDiffusionHalftoning() {}

   virtual bool IsProcessLineSupported() { return true; }

   virtual void ProcessLine(const buffer_t& InputLine, buffer_t& OutputLine)
   {
      int pixelValue = 0;
      int error = 0;
      size_t i = 0;

      // Set image  width
      if(!_imageWidth)
         _imageWidth = CalcImageWidth(InputLine);

      // Check buffer size
      OutputLine.resize(CalcOutputBufferSize(_imageWidth));
      std::fill(OutputLine.begin(), OutputLine.end(), byte(0));

      // Initialize halftone errors array and line buffer
      if(_error.size() == 0)
         _error.resize(_imageWidth, 0);

      if(_grayLine.size() == 0)
         _grayLine.resize(_imageWidth, 0);

      // Convert from RGB to GrayScale
      for(i = 0; i < _imageWidth; ++i)
      {
         byte R, G, B;
         ExtractRGB(InputLine, i, R, G, B);
         _grayLine[i] = RGBToGrayScale(R, G, B);
      }

      // Apply errors from prev line
      for(i = 0; i < _imageWidth; ++i)
      {
         // Only if not black and white
         if((_grayLine[i] != 0) && (_grayLine[i] != 255))
         {
            if(_error[i] + _grayLine[i] >= 255)
               _grayLine[i] = 255;
            else
               if(_error[i] + _grayLine[i] <= 0)
                  _grayLine[i] = 0;
               else
                  _grayLine[i] += _error[i];
         }

         _error[i] = 0;
      }


      // Compute output pixels and new errors
      for(i = 0; i < _imageWidth; ++i)
      {
         pixelValue = _grayLine[i] >= 128;
         error = _grayLine[i] - pixelValue * 255;

         if(_usePrinterColorSpace)
            SetPixelBW(OutputLine, i, !pixelValue);
         else
            SetPixelBW(OutputLine, i, pixelValue);

         // Disribute error
         if(i > 0)
            _error[i - 1] += (error * 3) / 16;

         _error[i] += (error * 5) / 16;

         if(i < _imageWidth - 1)
         {
            _error[i + 1] += (error * 1) / 16;

            if((_grayLine[i + 1] != 0) && (_grayLine[i + 1] != 255))
            {
               error = (error * 7) / 16;

               if(error + _grayLine[i + 1] >= 255)
                  _grayLine[i + 1] = 255;
               else
                  if(error + _grayLine[i + 1] <= 0)
                     _grayLine[i + 1] = 0;
                  else
                     _grayLine[i + 1] += error;
            }
         }
      } // for all pixels
   }

   virtual void ProcessImage(const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
   {
      OutputImage.clear();

      buffer_t InputLine;
      size_t BufferSize = CalcBufferSize(ImageWidth);
      InputLine.resize(BufferSize, 0);

      for(size_t i = 0; i < ImageHeight; ++i)
      {
         InputLine.assign((byte*)ImageData + LineDelta * i, (byte*)ImageData + LineDelta * i + BufferSize);
         OutputImage.push_back(buffer_t());
         ProcessLine(InputLine, OutputImage[OutputImage.size() - 1]);
      }
   }

   virtual void ProcessImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
   {
      OutputImage.clear();

      buffer_t OutputLine;

      for(std::vector<buffer_t>::const_iterator i = InputImage.begin(); i < InputImage.end(); ++i)
      {
         ProcessLine(*i, OutputLine);
         OutputImage.push_back(OutputLine);
      }
   }

protected:
   size_t GetImageWidth() { return _imageWidth; }

private:
   size_t _imageWidth;         // image width in pixels
   std::vector<int> _error;    // error buffer
   std::vector<int> _grayLine; // current line in gray scale color
   bool _usePrinterColorSpace; // if true then use 1 as black, 0 - as white; otherwise 0 is black 1 - white
};

};

#endif // ERROR_DIFFUSION_HALFTONING_H
