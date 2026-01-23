#include "NonLinearLaplacianHalftoning.h"
#include <algorithm>

namespace DymoPrinterDriver
{

// helper class defines image 'block' of 18 pixels
class NLLBlock
{
public:
  // create a block from image with coordinates of pixel #1 at (x1, y1)
  NLLBlock(NLLHalftoning& Parent, const HalftoneFilter::image_buffer_t& Image, int x1, int y1, HalftoneFilter::image_buffer_t& OutputImage);

  // return true if at least on pixels of the block is insize the image
  bool isInImage();

  // fill block information
  void fillBlock();
  void outputBlock();

private:
  // return intense value of the block - original number of pixels to draw in 'black'
  size_t getBlockIntenseValue();

  // fill info for one of 18 pixels
  // PixelNo - ordinal number of the pixel in the block
  // (x, y)  - coords of the pixel in original image
  void fillPixel(size_t PixelNo, int x, int y);

  // Split class 1 pixels to class 2 and class 5 to class 4
  void reduceClasses();
  void reduceClasses(size_t ClassFrom, size_t ClassTo);

  // return Laplacian value for pixel with coords (x, y)
  int calculateNll(int x, int y);

  // return grayscale value [0, 255] of pixel with coords (x, y)
  int getPixelGray(int x, int y);

  // output Pixels of specific class
  // return number of pixels drawn
  size_t  outputClass(size_t ClassNo, size_t MaxPixelsToOutput);

  void outputPixel(size_t PixelNo);
  void outputPixel(int x, int y);

  // return true if pixel (x, y) is inside image
  bool isInImage(int x, int y);

  NLLHalftoning&                         Parent_;
  const HalftoneFilter::image_buffer_t&  Image_;
  HalftoneFilter::image_buffer_t&        OutputImage_;
  int                                     x1_;
  int                                     y1_;

  std::vector<int>                        Pixels_;   // pixels' gray values
  std::vector<size_t>                     Classes_;  // pixels' classes
  size_t                                  ImageWidth_;
  size_t                                  ImageHeight_;

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

  static const point_t        PixelOffsets_[18];
  static const square_block_t Squares_[8];
};




const NLLBlock::point_t NLLBlock::PixelOffsets_[18] =
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

const NLLBlock::square_block_t NLLBlock::Squares_[8] =
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



NLLHalftoning::NLLHalftoning(int Threshold, image_t InputImageType, image_t OutputImageType):
  HalftoneFilter(InputImageType, OutputImageType), Threshold_(Threshold)
{
  if (getOutputImageType() != itBW)
    throw EHalftoneError(EHalftoneError::heUnsupportedImageType);
}

NLLHalftoning::~NLLHalftoning()
{
}

bool
NLLHalftoning::isProcessLineSupported()
{
  return false;
}

void
NLLHalftoning::processLine(
  const buffer_t& InputLine, buffer_t& OutputLine)
{
}

void
NLLHalftoning::processImage(
  const void* ImageData, size_t ImageWidth, size_t ImageHeight, size_t LineDelta, std::vector<buffer_t>& OutputImage)
{
  // TODO: non-implemented yet
}



void
NLLHalftoning::processImage(const std::vector<buffer_t>& InputImage, std::vector<buffer_t>& OutputImage)
{
  OutputImage.clear();
  if (InputImage.size() == 0) return;

  ImageWidth_   = calcImageWidth(InputImage[0]);
  ImageHeight_  = InputImage.size();

  // create an empty output image
  buffer_t EmptyLine;
  EmptyLine.resize(ImageWidth_ / 8 + 1, 0);
  OutputImage.resize(ImageHeight_, EmptyLine);

  // split the image to 18-pixels block
  size_t RowCount = (InputImage.size() + 1) / 3 + 1;
  for (size_t r = 0; r < RowCount; ++r)
  {
    // get coordinates of pixel #1
    size_t x1 = (r % 2) ? 3 : 0;
    size_t y1 = 3 * r;

    // for all blocks in the row
    // both leftest and rightest pixels of the block is
    while ((x1 - 3 < ImageWidth_) || (x1 + 2 < ImageWidth_))
    {
      NLLBlock Block(*this, InputImage, x1, y1, OutputImage);

      Block.fillBlock();
      Block.outputBlock();


      // advance to next block
      x1 += 6;
    } // for blocks in the row
  } // for rows
}


bool
NLLHalftoning::processDiagonal(
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
    NLLBlock Block(*this, InputImage, x, y, OutputImage);

    if (Block.isInImage())
    {
      //fprintf(stderr, "down Block (%i, %i)\n", x, y);
      Block.fillBlock();
      Block.outputBlock();
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
    NLLBlock Block(*this, InputImage, x, y, OutputImage);

    if (Block.isInImage())
    {
      //fprintf(stderr, "up Block (%i, %i)\n", x, y);

      Block.fillBlock();
      Block.outputBlock();
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
NLLHalftoning::getThreshold()
{
  return Threshold_;
}

////////////////////////////////////////////////////////////////////////
// Block class methods
////////////////////////////////////////////////////////////////////////

NLLBlock::NLLBlock(
  NLLHalftoning& parent, const HalftoneFilter::image_buffer_t& image, int x1, int y1, HalftoneFilter::image_buffer_t& output_image):
  Parent_(parent), Image_(image), OutputImage_(output_image), x1_(x1), y1_(y1), Pixels_(18, 0), Classes_(18, 0)
{
  ImageWidth_   = Parent_.calcImageWidth(Image_[0]);
  ImageHeight_  = Image_.size();
}

void
NLLBlock::fillBlock()
{
  for (size_t i = 0; i < Pixels_.size(); ++i)
    fillPixel(i + 1, x1_ + PixelOffsets_[i].x, y1_ + PixelOffsets_[i].y);

  reduceClasses();

}

void
NLLBlock::reduceClasses()
{
  //reduceClasses(1, 2);
  //reduceClasses(5, 4);
}

void
NLLBlock::reduceClasses(size_t class_from, size_t class_to)
{
  for (size_t i = 0; i < 8; ++i)
  {
    if ((Classes_[Squares_[i].p1 - 1] == class_from)
    && (Classes_[Squares_[i].p2 - 1] == class_from)
    && (Classes_[Squares_[i].p3 - 1] == class_from)
    && (Classes_[Squares_[i].p4 - 1] == class_from))
    {
      Classes_[Squares_[i].p1 - 1] = class_to;
      Classes_[Squares_[i].p3 - 1] = class_to;
    }
  }
}

void
NLLBlock::fillPixel(size_t pixel_no, int x, int y)
{
  // fill pixels
  Pixels_[pixel_no - 1] = getPixelGray(x, y);

  // fill classes
  int nll = calculateNll(x, y);
  int threshold = Parent_.getThreshold();

  // we added to new classes to those described in the papers
  // 0 - (same as 1) - it is set for black pixels, those should remeined black
  // 6 - (same as 5) - it is set for white pixels, those should remeined white

  if (Pixels_[pixel_no - 1] == 0)
    Classes_[pixel_no - 1] = 0;
  else if (Pixels_[pixel_no - 1] == 255)
    Classes_[pixel_no - 1] = 6;
  else // as in papers
    if (nll < -threshold)
      Classes_[pixel_no - 1] = 1;
    else if (nll > threshold)
      Classes_[pixel_no - 1] = 5;
    else
      Classes_[pixel_no - 1] = 3;

}

int
NLLBlock::getPixelGray(int x, int y)
{
  if (isInImage(x, y))
  {
    byte R, G, B;
    Parent_.extractRGB(Image_[y], x, R, G, B);
    return Parent_.convertRGBToGrayScale(R, G, B);
  }
  else
    return 255; // white
}

int
NLLBlock::calculateNll(int x, int y)
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
NLLBlock::getBlockIntenseValue()
{
  size_t Intense = 128;
  for (size_t i = 0; i < Pixels_.size(); ++i)
    Intense += Pixels_[i];

  size_t result =  18 - std::min(Intense / 255, size_t(18));
  //fprintf(stderr, "getBlockIntenseValue() = %d\n", result);
  return result;
}

void
NLLBlock::outputBlock()
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
NLLBlock::outputClass(size_t class_no, size_t max_pixels_to_output)
{
  //fprintf(stderr, "outputClass(%i, %i)\n", class_no, max_pixels_to_output);

  if (max_pixels_to_output == 0)
    return 0;

  std::vector<size_t> Pixels;

  // collect all pixels of a class
  for (size_t i = 0; i < Classes_.size(); ++i)
    if (Classes_[i] == class_no)
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
NLLBlock::outputPixel(size_t pixel_no)
{
  outputPixel(x1_ + PixelOffsets_[pixel_no - 1].x, y1_ + PixelOffsets_[pixel_no - 1].y);
}

void
NLLBlock::outputPixel(int x, int y)
{
  if (isInImage(x, y))
    Parent_.setPixelBW(OutputImage_[y], x, 1);
}

bool
NLLBlock::isInImage()
{
  for (size_t i = 0; i < Pixels_.size(); ++i)
    if (isInImage(x1_ + PixelOffsets_[i].x, y1_ + PixelOffsets_[i].y))
      return true;

  return false;
}

bool
NLLBlock::isInImage(int x, int y)
{
  return (x >= 0) && (size_t(x) < ImageWidth_) && (y >= 0) && (size_t(y) < ImageHeight_);
}


} // namespace
