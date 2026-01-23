#include "NonLinearLaplacianHalftoning.h"
#include <algorithm>

namespace DymoPrinterDriver
{

// helper class defines image 'block' of 18 pixels
class NonLinearLaplacianBlock
{
public:
  // create a block from image with coordinates of pixel #1 at (x1, y1)
  NonLinearLaplacianBlock(NonLinearLaplacianHalftoning& parent, const HalftoneFilter::image_buffer_t& image, int x1, int y1, HalftoneFilter::image_buffer_t& outputImage);

  // return true if at least on pixels of the block is insize the image
  bool isInImage();

  // fill block information
  void fillBlock();
  void outputBlock();

private:
  // return intense value of the block - original number of pixels to draw in 'black'
  size_t getBlockIntenseValue();

  // fill info for one of 18 pixels
  // pixelNumber - ordinal number of the pixel in the block
  // (x, y)  - coordinates of the pixel in original image
  void fillPixel(size_t pixelNumber, int x, int y);

  // Split class 1 pixels to class 2 and class 5 to class 4
  void reduceClasses();
  void reduceClasses(size_t classFrom, size_t classTo);

  // return Laplacian value for pixel with coordinates (x, y)
  int calculateNonLinearLaplacian(int x, int y);

  // return grayscale value [0, 255] of pixel with coordinates (x, y)
  int getPixelGray(int x, int y);

  // output Pixels of specific class
  // return number of pixels drawn
  size_t  outputClass(size_t classNumber, size_t maxPixelsToOutput);

  void outputPixel(size_t pixelNumber);
  void outputPixel(int x, int y);

  // return true if pixel (x, y) is inside image
  bool isInImage(int x, int y);

  NonLinearLaplacianHalftoning&                         parent;
  const HalftoneFilter::image_buffer_t&  image;
  HalftoneFilter::image_buffer_t&        outputImage;
  int                                     x1;
  int                                     y1;

  std::vector<int>                        pixels;   // pixels' gray values
  std::vector<size_t>                     classes;  // pixels' classes
  size_t                                  imageWidth;
  size_t                                  imageHeight;

  typedef struct
  {
    int x;
    int y;
  } point_t;

  typedef struct
  {
    size_t p1;
    size_t p2;
    size_t p3;
    size_t p4;
  } square_block_t;

  static const point_t        pixelOffsets[18];
  static const square_block_t squares[8];
};




const NonLinearLaplacianBlock::point_t NonLinearLaplacianBlock::pixelOffsets[18] =
{
  { 0,  0},
  {-1,  1},
  {-1, -1},
  { 1, -1},
  { 1,  1},
  {-2,  0},
  { 2,  0},
  { 0, -2},
  { 0,  2},
  {-1,  0},
  {-2, -1},
  {-2,  1},
  { 0, -1},
  { 0,  1},
  {-3,  0},
  {-1, -2},
  {-1,  2},
  { 1,  0},
};

const NonLinearLaplacianBlock::square_block_t NonLinearLaplacianBlock::squares[8] =
{
  {17,  8,  3, 13},
  {11,  3,  6, 10},
  { 3, 13, 10,  1},
  {13,  4,  1, 18},
  { 6, 10, 12,  2},
  {10,  1,  2, 14},
  { 1, 18, 14,  5},
  { 2, 14, 16,  9}
};



NonLinearLaplacianHalftoning::NonLinearLaplacianHalftoning(int threshold, image_t inputImageType, image_t outputImageType):
  HalftoneFilter(inputImageType, outputImageType), threshold(threshold)
{
  if (getOutputImageType() != itBW)
    throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
}

NonLinearLaplacianHalftoning::~NonLinearLaplacianHalftoning()
{
}

bool
NonLinearLaplacianHalftoning::isProcessLineSupported()
{
  return false;
}

void
NonLinearLaplacianHalftoning::processLine(
  const buffer_t& InputLine, buffer_t& OutputLine)
{
}

void
NonLinearLaplacianHalftoning::processImage(
  const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
{
  // TODO: non-implemented yet
}



void
NonLinearLaplacianHalftoning::processImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();
  if (InputImage.size() == 0) return;

  imageWidth   = calcImageWidth(InputImage[0]);
  imageHeight  = InputImage.size();

  // create an empty output image
  buffer_t emptyLine;
  emptyLine.resize(imageWidth / 8 + 1, 0);
  OutputImage.resize(imageHeight, emptyLine);

  // split the image to 18-pixels block
  size_t RowCount = (InputImage.size() + 1) / 3 + 1;
  for (size_t r = 0; r < RowCount; ++r)
  {
    // get coordinates of pixel #1
    size_t x1 = (r % 2) ? 3 : 0;
    size_t y1 = 3 * r;

    // for all blocks in the row
    // both leftest and rightest pixels of the block is
    while ((x1 - 3 < imageWidth) || (x1 + 2 < imageWidth))
    {
      NonLinearLaplacianBlock block(*this, InputImage, x1, y1, OutputImage);

      block.fillBlock();
      block.outputBlock();


      // advance to next block
      x1 += 6;
    } // for blocks in the row
  } // for rows
}


bool
NonLinearLaplacianHalftoning::processDiagonal(
  const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage, size_t& x1, size_t& y1)
{
  //fprintf(stderr, "processDiagonal(%i, %i)\n", x1, y1);

  bool   Result           = false;
  bool   HasDownBlocks    = false;
  bool   HasUpBlocks      = false;
  size_t UpperDownX       = 0;
  size_t UpperDownY       = 0;

  // go down
  size_t x            = x1 - 3;
  size_t y            = y1 + 3;
  while (true)
  {
    NonLinearLaplacianBlock block(*this, InputImage, x, y, OutputImage);

    if (block.isInImage())
    {
      //fprintf(stderr, "down Block (%i, %i)\n", x, y);
      block.fillBlock();
      block.outputBlock();
      Result = true;

      if (!HasDownBlocks)
      {
        UpperDownX = x;
        UpperDownY = y;
        HasDownBlocks = true;
      }

      x -= 3;
      y += 3;
    }
    else
      break;
  } // go down

    // go up
  x   = x1;
  y   = y1;
  while (true)
  {
    NonLinearLaplacianBlock block(*this, InputImage, x, y, OutputImage);

    if (block.isInImage())
    {
      //fprintf(stderr, "up Block (%i, %i)\n", x, y);

      block.fillBlock();
      block.outputBlock();
      Result = true;
      HasUpBlocks = true;

      x1   = x;
      y1   = y;
      x += 3;
      y -= 3;
    }
    else
      break;
  } // go up

  if (Result && !HasUpBlocks)
  {
    x1 = UpperDownX;
    y1 = UpperDownY;
  }
  //fprintf(stderr, "ProcessDiagonal returns (%i, %i)\n", x1, y1);

  return Result;
}

int
NonLinearLaplacianHalftoning::getThreshold()
{
  return threshold;
}

////////////////////////////////////////////////////////////////////////
// Block class methods
////////////////////////////////////////////////////////////////////////

NonLinearLaplacianBlock::NonLinearLaplacianBlock(
  NonLinearLaplacianHalftoning& parent, const HalftoneFilter::image_buffer_t& image, int x1, int y1, HalftoneFilter::image_buffer_t& outputImage):
  parent(parent), image(image), outputImage(outputImage), x1(x1), y1(y1), pixels(18, 0), classes(18, 0)
{
  imageWidth   = parent.calcImageWidth(image[0]);
  imageHeight  = image.size();
}

void
NonLinearLaplacianBlock::fillBlock()
{
  for (size_t i = 0; i < pixels.size(); ++i)
    fillPixel(i + 1, x1 + pixelOffsets[i].x, y1 + pixelOffsets[i].y);

  reduceClasses();

}

void
NonLinearLaplacianBlock::reduceClasses()
{
  //reduceClasses(1, 2);
  //reduceClasses(5, 4);
}

void
NonLinearLaplacianBlock::reduceClasses(size_t class_from, size_t class_to)
{
  for (size_t i = 0; i < 8; ++i)
  {
    if ((classes[squares[i].p1 - 1] == class_from)
    && (classes[squares[i].p2 - 1] == class_from)
    && (classes[squares[i].p3 - 1] == class_from)
    && (classes[squares[i].p4 - 1] == class_from))
    {
      classes[squares[i].p1 - 1] = class_to;
      classes[squares[i].p3 - 1] = class_to;
    }
  }
}

void
NonLinearLaplacianBlock::fillPixel(size_t pixelNumber, int x, int y)
{
  // fill pixels
  pixels[pixelNumber - 1] = getPixelGray(x, y);

  // fill classes
  int nll = calculateNonLinearLaplacian(x, y);
  int threshold = parent.getThreshold();

  // we added to new classes to those described in the papers
  // 0 - (same as 1) - it is set for black pixels, those should remeined black
  // 6 - (same as 5) - it is set for white pixels, those should remeined white

  if (pixels[pixelNumber - 1] == 0)
    classes[pixelNumber - 1] = 0;
  else if (pixels[pixelNumber - 1] == 255)
    classes[pixelNumber - 1] = 6;
  else // as in papers
    if (nll < -threshold)
      classes[pixelNumber - 1] = 1;
    else if (nll > threshold)
      classes[pixelNumber - 1] = 5;
    else
      classes[pixelNumber - 1] = 3;

}

int
NonLinearLaplacianBlock::getPixelGray(int x, int y)
{
  if (isInImage(x, y))
  {
    byte R, G, B;
    parent.extractRGB(image[y], x, R, G, B);
    return parent.convertRGBToGrayScale(R, G, B);
  }
  else
    return 255; // white
}

int
NonLinearLaplacianBlock::calculateNonLinearLaplacian(int x, int y)
{
  int A =
    getPixelGray(x, y)
    - (  getPixelGray(x - 1, y - 1)
    + getPixelGray(x + 1, y - 1)
    + getPixelGray(x - 1, y + 1)
    + getPixelGray(x + 1, y + 1)) / 4;

  int B =
    getPixelGray(x, y)
    - (  getPixelGray(x - 0, y - 1)
    + getPixelGray(x + 0, y + 1)
    + getPixelGray(x - 1, y + 0)
    + getPixelGray(x + 1, y + 0)) / 4;

  if ((A > 0) && (B > 0))
    return std::min(A, B);

  if ((A < 0) && (B < 0))
    return -std::min(abs(A), abs(B));

  return 0;
}


size_t
NonLinearLaplacianBlock::getBlockIntenseValue()
{
  size_t intense = 128;
  for (size_t i = 0; i < pixels.size(); ++i)
    intense += pixels[i];

  size_t result =  18 - std::min(intense / 255, size_t(18));
  //fprintf(stderr, "getBlockIntenseValue() = %d\n", result);
  return result;
}

void
NonLinearLaplacianBlock::outputBlock()
{
  size_t RemainedPixels = getBlockIntenseValue();
  size_t PixelCount       = 0;

  // output all pixels for class 0
  PixelCount = outputClass(0, 18);

  if (PixelCount < RemainedPixels)
  {
    RemainedPixels -= PixelCount;
    RemainedPixels -= outputClass(1, RemainedPixels);
    RemainedPixels -= outputClass(2, RemainedPixels);
    RemainedPixels -= outputClass(3, RemainedPixels);
    RemainedPixels -= outputClass(4, RemainedPixels);
  }
}

size_t
NonLinearLaplacianBlock::outputClass(size_t classNumber, size_t max_pixels_to_output)
{
  //fprintf(stderr, "outputClass(%i, %i)\n", classNumber, max_pixels_to_output);

  if (max_pixels_to_output == 0)
    return 0;

  std::vector<size_t> Pixels;

  // collect all pixels of a class
  for (size_t i = 0; i < classes.size(); ++i)
    if (classes[i] == classNumber)
      Pixels.push_back(i + 1);

  // sort pixels, so output it in order
  //std::sort(Pixels.begin(), Pixels.end());
  //std::random_shuffle(Pixels.begin(), Pixels.end());

  // output pixels
  size_t Result = 0;
  for (size_t i = 0; i < Pixels.size(); ++i)
    if (Result < max_pixels_to_output)
    {
      outputPixel(Pixels[i]);
      ++Result;
    }
    else
      break;

  return Result;
}

void
NonLinearLaplacianBlock::outputPixel(size_t pixelNumber)
{
  outputPixel(x1 + pixelOffsets[pixelNumber - 1].x, y1 + pixelOffsets[pixelNumber - 1].y);
}

void
NonLinearLaplacianBlock::outputPixel(int x, int y)
{
  if (isInImage(x, y))
    parent.setPixelBW(outputImage[y], x, 1);
}

bool
NonLinearLaplacianBlock::isInImage()
{
  for (size_t i = 0; i < pixels.size(); ++i)
    if (isInImage(x1 + pixelOffsets[i].x, y1 + pixelOffsets[i].y))
      return true;

  return false;
}

bool
NonLinearLaplacianBlock::isInImage(int x, int y)
{
  return (x >= 0) && (size_t(x) < imageWidth) && (y >= 0) && (size_t(y) < imageHeight);
}


} // namespace
