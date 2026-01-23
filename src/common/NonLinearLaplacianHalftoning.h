#ifndef NONLINEAR_LAPLACIAN_HALFTONING_H
#define NONLINEAR_LAPLACIAN_HALFTONING_H

#include "Halftoning.h"

namespace DymoPrinterDriver
{

class NonLinearLaplacianHalftoning: public HalftoneFilter
{
public:
  NonLinearLaplacianHalftoning(int threshold, image_t input_image_type, image_t output_image_type);
  virtual ~NonLinearLaplacianHalftoning();

  virtual bool isProcessLineSupported();
  virtual void processLine(const buffer_t& input_line, buffer_t& output_line);
  virtual void processImage(const void* image_data, size_t image_width, size_t image_height, size_t line_delta, std::vector<buffer_t>& output_image);
  virtual void processImage(const image_buffer_t& input_image, image_buffer_t& output_image);

  int getThreshold();
protected:
private:
  int threshold;  // constant used to separate a block to classes using NonLinearLaplacian

  size_t imageWidth;
  size_t imageHeight;

  // split image to 18-pixels block be diagonal
  // return true if diagonal contains at least one Block inside image, so next diagonal should be processes
  // on output (x1, y1) is coordinates of pixel #1 of topmost block in the diagonal
  bool processDiagonal(
    const std::vector<buffer_t>& input_image, std::vector<buffer_t>& output_image, size_t& x1, size_t& y1);

};

}; // namespace

#endif // NONLINEAR_LAPLACIAN_HALFTONING_H
