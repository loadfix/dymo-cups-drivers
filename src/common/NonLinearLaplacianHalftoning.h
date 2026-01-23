#ifndef NONLINEAR_LAPLACIAN_HALFTONING_H
#define NONLINEAR_LAPLACIAN_HALFTONING_H

#include "Halftoning.h"

namespace DymoPrinterDriver
{

class NLLHalftoning: public HalftoneFilter
{
public:
  NLLHalftoning(int Threshold, image_t InputImageType, image_t OutputImageType);
  virtual ~NLLHalftoning();

  virtual bool IsProcessLineSupported();
  virtual void ProcessLine(const buffer_t& InputLine, buffer_t& OutputLine);
  virtual void ProcessImage(const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage);
  virtual void ProcessImage(const image_buffer_t& InputImage, image_buffer_t& OutputImage);

  int GetThreshold();
protected:
private:
  int Threshold_;  // constant used to separate a block to classes using NLL

  size_t ImageWidth_;
  size_t ImageHeight_;

  // split image to 18-pixels block be diagonal
  // return true if diagonal contains at least one Block inside image, so next diagonal should be processes
  // on output (x1, y1) is coodrs of pixel #1 of topmost block in the diagonal
  bool ProcessDiagonal(
    const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage, size_t& x1, size_t& y1);

};

}; // namespace

#endif // NONLINEAR_LAPLACIAN_HALFTONING_H
