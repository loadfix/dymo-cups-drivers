#ifndef HALFTONING_H
#define HALFTONING_H

#include <stdlib.h>
#include "CommonTypedefs.h"

namespace DymoPrinterDriver
{

class HalftoneFilter
{
public:
   // image format
   typedef enum
   {
      itBW,     // one bit per pixel, black and white
      itXRGB,   // four bytes per pixel, 8 bits per color, msb is not used (default on MacOSX)
      itRGB,    // three bytes per pixel, 8 bits per color (default on CUPS)
   } image_t;

   typedef std::vector<buffer_t> image_buffer_t;

   HalftoneFilter(image_t input_image_type, image_t output_image_type) : _inputImageType(input_image_type), _outputImageType(output_image_type) {}
   virtual ~HalftoneFilter() {}

   // Line-by-line interface
   virtual bool isProcessLineSupported() = 0;
   virtual void processLine(const buffer_t& input_line, buffer_t& output_line) = 0;

   // Full-image-at-once interface
   virtual void processImage(const void* image_data, size_t image_width, size_t image_height, size_t line_delta, std::vector<buffer_t>& output_image) = 0;
   virtual void processImage(const image_buffer_t& input_image, image_buffer_t& output_image) = 0;

   image_t getInputImageType() { return _inputImageType; }
   image_t getOutputImageType() { return _outputImageType; }

   // Convert RGB value to Gray Scale
   byte convertRGBToGrayScale(byte r, byte g, byte b)
   {
      // White should remain white
      if((r == 255) && (g == 255) && (b == 255))
         return 255;
      // black should remain black
      else if((r == 0) && (g == 0) && (b == 0))
         return 0;
      // and if gray scale then keep it
      else if((r == g) && (g == b))
         return r;
      else
      {
         int r_val = 0 + ((int(r) * 299) / 1000) + ((int(g) * 587) / 1000) + ((int(b) * 114) / 1000);
         if(r_val > 255)
            return 255;
         return byte(r_val);
      }
   }

   // PixelValue (0 - white, 1 - black)
   void setPixelBW(buffer_t& buf, int pixel_no, int pixel_value)
   {
      if(pixel_value)
         buf[pixel_no / 8] |= (1 << (7 - pixel_no % 8));
      else
         buf[pixel_no / 8] &= ~(1 << (7 - pixel_no % 8));
   }

   // Based on inputImageType extract color component of current pixel
   void extractRGB(const buffer_t& input_line, int pixel_no, byte& r, byte& g, byte& b)
   {
      switch(_inputImageType)
      {
         case itXRGB:
            r = input_line[4 * pixel_no + 1];
            g = input_line[4 * pixel_no + 2];
            b = input_line[4 * pixel_no + 3];
            break;
         case itRGB:
            r = input_line[3 * pixel_no + 0];
            g = input_line[3 * pixel_no + 1];
            b = input_line[3 * pixel_no + 2];
            break;
         default:
            // We shouldn't come here!
            break;
      }
   }

   // Same as previous but return colors as packed integer value
   int extractRGB(const buffer_t& input_line, int pixel_no)
   {
      switch(_inputImageType)
      {
         case itXRGB:
            return (int(input_line[4 * pixel_no + 1]) << 16)
                   | (int(input_line[4 * pixel_no + 2]) << 8)
                   | (input_line[4 * pixel_no + 3]);
         case itRGB:
            return (int(input_line[3 * pixel_no + 0]) << 16)
                   | (int(input_line[3 * pixel_no + 1]) << 8)
                   | (input_line[3 * pixel_no + 2]);
         default:
            // We shouldn't come here!
            return -1;
      }

      return 0;
   }

   // Return imageWidth based on inputImageType and input line data
   size_t calcImageWidth(const buffer_t& input_line)
   {
      switch(_inputImageType)
      {
         case itXRGB:
            return input_line.size() / 4;
         case itRGB:
            return input_line.size() / 3;
         default:
            // We shouldn't come here!
            return 0;
      }

      return 0;
   }

   // Return buffer size needed to store an input line based on inputImageType
   size_t calcBufferSize(size_t image_width)
   {
      switch(_inputImageType)
      {
         case itXRGB:
            return image_width * 4;
         case itRGB:
            return image_width * 3;
         default:
            // We shouldn't come here!
            return 0;
      }

      return 0;
   }

   // Calc output buffer size
   size_t calcOutputBufferSize(size_t image_width)
   {
      switch(_outputImageType)
      {
         case itBW:
            if(image_width % 8 == 0)
               return image_width / 8;
            else
               return image_width / 8 + 1;
         default:
            // We shouldn't come here!
            return 0;
      }

      return 0;
   }

private:
  image_t _inputImageType;
  image_t _outputImageType;
};

// Error Halftoning
class EHalftoneError
{
public:
   typedef enum
   {
      heUnsupportedImageType = 1,
   } error_t;

   EHalftoneError(error_t error_code) : _errorCode(error_code) {}

   error_t getErrorCode() { return _errorCode; }

private:
   error_t _errorCode;
};

}

#endif // HALFTONING_H
