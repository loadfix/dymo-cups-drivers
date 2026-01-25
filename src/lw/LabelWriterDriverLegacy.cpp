namespace DymoPrinterDriver
{


const byte ESC = 0x1B;
const byte SYN = 0x16;
const byte ETB = 0x17;

LabelWriterDriver::LabelWriterDriver(IPrintEnvironment& environment):
  environment(environment),
  resolution(RESOLUTION_UNKNOWN), density(PRINT_DENSITY_NORMAL), quality(PRINT_QUALITY_TEXT), pageHeight(0x0800), paperType(PAPER_TYPE_REGULAR),
  maxPrintWidth(84),pageOffset(0, 0),lastDotTab(size_t(-1)), lastBytesPerLine(size_t(-1)), emptyLinesCount(0)
{
}

LabelWriterDriver::~LabelWriterDriver()
{
}

void
LabelWriterDriver::startDoc()
{
  sendCommand(getResetCommand());
  sendResolution(resolution);
  sendLineTab(0);
  sendDotTab(0);
  sendPrintQuality(quality);
  sendPrintDensity(density);
}

void
LabelWriterDriver::endDoc()
{
}

void
LabelWriterDriver::startPage()
{
  switch (paperType)
  {
    case PAPER_TYPE_REGULAR:      sendLabelLength(pageHeight); break;
    case PAPER_TYPE_CONTINUOUS:   sendLabelLength(0xffff); break;
    default:        assert(0);
  }

  lastDotTab = size_t(-1);
  lastBytesPerLine = size_t(-1);
  emptyLinesCount = 0;
}

void
LabelWriterDriver::endPage()
{
  sendFormFeed();
}


void
LabelWriterDriver::sendNotCompressedData(
  const buffer_t& buffer, size_t leaderBlanks, size_t trailerBlanks)
{
  byte syn = SYN;

  size_t dataSize = buffer.size() - leaderBlanks - trailerBlanks;

  // set bytes per line in case of it changes from last raster line
  if (lastBytesPerLine != dataSize)
  {
    sendBytesPerLine(dataSize);
    lastBytesPerLine = dataSize;
  }

  sendCommand(&syn, sizeof(syn));
  sendCommand(&buffer[0] + leaderBlanks, dataSize);
}

void
LabelWriterDriver::sendCompressedData(
  const buffer_t& compressedBuffer, size_t notCompressedSize)
{
  byte etb = ETB;

  // set bytes per line in case of it changes from last raster line
  if (lastBytesPerLine != notCompressedSize)
  {
    sendBytesPerLine(notCompressedSize);
    lastBytesPerLine = notCompressedSize;
  }

  sendCommand(&etb, sizeof(etb));
  sendCommand(&compressedBuffer[0], compressedBuffer.size());
}

void
LabelWriterDriver::getBlanks(
  const buffer_t& buffer, size_t& leaderBlanks, size_t& trailerBlanks)
{
  size_t i = 0;

  leaderBlanks    = 0;
  trailerBlanks   = 0;

  size_t bufferSize = buffer.size();

  // count left spaces
  for (i = 0; i < bufferSize; ++i)
    if (buffer[i] == 0)
      ++leaderBlanks;
    else
      break;

  if (i == bufferSize) return;

  // count right spaces
  for (i = bufferSize - 1; i >= 0; --i)
    if (buffer[i] == 0)
      ++trailerBlanks;
    else
      break;
} // GetBlanks()

// bit numbers in byte
// 7 6 5 4 3 2 1 0
// msb           lsb

// Returns value of bit BitNo in byte Data
// if bit unset returns 0, else - not 0
static inline byte
GetBitValue(byte data, size_t bitNumber)
{
  return data & (1 << bitNumber);
}

// Advanses to one bit in byte sequence
static inline void
NextBit(size_t& currentByteNumber, size_t& currentBitNumber)
{
  if (currentBitNumber == 0)
  {
    currentByteNumber = currentByteNumber + 1;
    currentBitNumber = 7;
  }
  else
    currentBitNumber = currentBitNumber - 1;
}

// Returns RLE compressed value for data in Data with size DataLen
// start compression at curByteNo/curBitNo
// At exit CureByteNo, curBitNo contains next bit after compressed sequence
static byte
GetCompressedSequenceValue(const byte* data, size_t dataLength, size_t& currentByteNumber, size_t& currentBitNumber)
{
  byte bitCount = 0;
  byte bitValue = 0;
  byte startBitValue = GetBitValue(data[currentByteNumber], currentBitNumber);
  NextBit(currentByteNumber, currentBitNumber);

  // while data exist and not max len of sequence
  while ((currentByteNumber < dataLength) && (bitCount < 0x7f))
  {
    bitValue = GetBitValue(data[currentByteNumber], currentBitNumber);

    // same as prev bit
    if ((startBitValue && bitValue) || (!startBitValue && !bitValue))
    {
      bitCount++;
      NextBit(currentByteNumber, currentBitNumber);
    }
    else // end of sequence
      break;
  } // while

  if (startBitValue)
    return bitCount | 0x80; // set high bit for "black" pixels
  else
    return bitCount;
}

static void
CompressData(buffer_t& compressedData, const byte* data, size_t dataSize)
{
  size_t currentByteNumber        = 0;
  size_t currentBitNumber         = 7;
  size_t compressedOffset = 0;

  while (currentByteNumber < dataSize)
  {
    if (compressedOffset >= dataSize - 1)
    {
      compressedData.clear(); // will write non-compressed data
      return;
    }
    compressedData.push_back(GetCompressedSequenceValue(data, dataSize, currentByteNumber, currentBitNumber));
    ++compressedOffset;
  }
}

static void
ShiftDataRight(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue)
{
  // shift bytes first
  int shiftedLength = shiftedBuffer.size() - shiftValue / 8;
  size_t shiftedOffset = shiftValue / 8;
  shiftValue   = shiftValue % 8;

  if ((shiftedLength <= 0) || (buffer.size() == 0)) return;

  // shift bits
  shiftedBuffer[shiftedOffset] = buffer[0] >> shiftValue; // first
  size_t i = 0;
  for (i = 1; ((i < buffer.size()) && (i < size_t(shiftedLength))); ++i)
    shiftedBuffer[shiftedOffset + i] = (buffer[i - 1] << (8 - shiftValue)) | (buffer[i] >> shiftValue);
  if (i < size_t(shiftedLength))
    shiftedBuffer[shiftedOffset + buffer.size()] = (buffer[buffer.size() - 1] << (8 - shiftValue));
}

static void
ShiftDataLeft(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue)
{
  // shift bytes first
  int shiftedLength = shiftedBuffer.size() - shiftValue / 8;
  shiftValue   = shiftValue % 8;

  if ((shiftedLength <= 0) || (buffer.size() == 0)) return;

  // shift bits
  size_t i = 0;
  for (i = 0; ((i < buffer.size() - 1) && (i < size_t(shiftedLength))); ++i)
    shiftedBuffer[i] = (buffer[i] << shiftValue) | (buffer[i + 1] >> (8 - shiftValue));
  if (i < size_t(shiftedLength))
    shiftedBuffer[buffer.size() - 1] = (buffer[buffer.size() - 1] << shiftValue); // last
}


static void
ShiftData(const buffer_t& buffer, buffer_t& shiftedBuffer, int shiftValue)
{
  // clear shift buffer first
  for (size_t i = 0; i < shiftedBuffer.size(); ++i)
    shiftedBuffer[i] = 0;

  if (shiftValue >= 0)
    ShiftDataRight(buffer, shiftedBuffer, shiftValue);
  else
    ShiftDataLeft(buffer, shiftedBuffer, -shiftValue);
}

void
LabelWriterDriver::processRasterLine(const buffer_t& lineBuffer)
{
  buffer_t b = lineBuffer;

  if (pageOffset.x > 0)
  {
    buffer_t b2(b.size() + (pageOffset.x + 7) / 8, 0);
    ShiftData(b, b2, pageOffset.x);
    b = b2;
  }

  if (b.size() > maxPrintWidth)
  {
    fputs("WARNING: LabelWriterDriver::ProcessRasterLine(): page width is greater max page width, truncated\n", stderr);
    b = buffer_t(b.begin(), b.begin() + maxPrintWidth);
  }

  size_t leaderBlanks = 0;
  size_t trailerBlanks = 0;

  // get blanks count
  getBlanks(b, leaderBlanks, trailerBlanks);

  if (leaderBlanks + trailerBlanks == b.size())
  {
    // remember empty line
    ++emptyLinesCount;
  }
  else // not empty line
  {
    // skip empty lines
    if (emptyLinesCount)
      sendSkipLines(emptyLinesCount);

    emptyLinesCount = 0;

    // set dot tab
    // Bug Fix for DLS80AM-1421
    // NOTE: an ESC B needs to be send for each raster line. Otherwise the LW 3xx series output
    // will be distorted.
    //if (lastDotTab != leaderBlanks)
    //{
      sendDotTab(leaderBlanks);
      lastDotTab = leaderBlanks;
    //}

    // calculate compressed data size
    buffer_t compressedData;
    CompressData(compressedData, &b[0] + leaderBlanks, b.size() - leaderBlanks - trailerBlanks);

    if ((compressedData.size() > 0) && (compressedData.size() < b.size() - leaderBlanks - trailerBlanks))
      sendCompressedData(compressedData, b.size() - leaderBlanks - trailerBlanks);
    else
      sendNotCompressedData(b, leaderBlanks, trailerBlanks);
  }


}


void
LabelWriterDriver::sendCommand(const byte* buffer, size_t bufferSize)
{
  environment.WriteData(buffer_t(buffer, buffer + bufferSize));
}

void
LabelWriterDriver::sendCommand(const buffer_t& buffer)
{
  environment.WriteData(buffer);
}

LabelWriterDriver::resolution_t
LabelWriterDriver::getResolution()
{
  return resolution;
}

LabelWriterDriver::density_t
LabelWriterDriver::getDensity()
{
  return density;
}

LabelWriterDriver::quality_t
LabelWriterDriver::getQuality()
{
  return quality;
}

size_t
LabelWriterDriver::getPageHeight()
{
  return pageHeight;
}

LabelWriterDriver::paper_type_t
LabelWriterDriver::getPaperType()
{
  return paperType;
}

void
LabelWriterDriver::setresolution(LabelWriterDriver::resolution_t value)
{
  resolution = value;
}

void
LabelWriterDriver::setdensity(LabelWriterDriver::density_t value)
{
  density = value;
}

void
LabelWriterDriver::setquality(LabelWriterDriver::quality_t value)
{
  quality = value;
}

void
LabelWriterDriver::setpageHeight(size_t value)
{
  pageHeight = value;
}

void
LabelWriterDriver::setpaperType(LabelWriterDriver::paper_type_t value)
{
  paperType = value;
}

void
LabelWriterDriver::setMaxPrintWidth(size_t value)
{
  maxPrintWidth = value;
}

void
LabelWriterDriver::setPageOffset(point_t value)
{
  pageOffset = value;
}

void
LabelWriterDriver::sendLineTab(size_t value)
{
  byte buffer[] = {ESC, 'Q', 0, 0};
  buffer[2] = (value >> 8) & 0xff;
  buffer[3] = value & 0xff;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendDotTab(size_t value)
{
  byte buffer[] = {ESC, 'B', 0};
  buffer[2] = value;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendFormFeed()
{
  byte buffer[] = {ESC, 'E'};

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendBytesPerLine(size_t value)
{
  byte buffer[] = {ESC, 'D', 0};
  buffer[2] = value;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendSkipLines(size_t value)
{
  const size_t MAX_LINES = 255;

  // a hardware can skip no more 255 lines at time
  byte buffer[] = {ESC, 'f', 1, 0};

  while (value > 0)
  {
    if (value > MAX_LINES)
    {
      buffer[3] = MAX_LINES;
      value -= MAX_LINES;
    }
    else
    {
      buffer[3] = value;
      value  = 0;
    }

    sendCommand(buffer, sizeof(buffer));
  }
}

void
LabelWriterDriver::sendLabelLength(size_t value)
{
  byte buffer[] = {ESC, 'L', 0, 0};
  buffer[2] = (value >> 8) & 0xff;
  buffer[3] = value & 0xff;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendResolution(resolution_t value)
{
  if (value == RESOLUTION_UNKNOWN)
    return;

  byte buffer[] = {ESC, 0};
  switch (value)
  {
    case RESOLUTION_136:
      buffer[1] = 'z';
      break;
    case RESOLUTION_204:
      buffer[1] = 'y';
      break;
    default:
      assert(0);
      break;
  }

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendPrintDensity(density_t value)
{
  byte buffer[] = {ESC, 'e'};

  switch (value)
  {
    case PRINT_DENSITY_LOW:     buffer[1] = 'c'; break;
    case PRINT_DENSITY_MEDIUM:  buffer[1] = 'd'; break;
    case PRINT_DENSITY_NORMAL:  buffer[1] = 'e'; break;
    case PRINT_DENSITY_HIGH:    buffer[1] = 'g'; break;
    default:        buffer[1] = 'e'; break; // normal
  }

  sendCommand(buffer, sizeof(buffer));
}

void
LabelWriterDriver::sendPrintQuality(quality_t value)
{
  byte buffer[] = {ESC, 'h'};

  switch (value)
  {
    case PRINT_QUALITY_TEXT:                buffer[1] = 'h'; break;
    case PRINT_QUALITY_BARCODE_AND_GRAPHICS:  buffer[1] = 'i'; break;
    default:                    buffer[1] = 'h'; break; // text
  }

  sendCommand(buffer, sizeof(buffer));
}

buffer_t
LabelWriterDriver::getResetCommand()
{
  return buffer_t(156, ESC);
}

buffer_t
LabelWriterDriver::getRequestStatusCommand()
{
  byte buffer[] = {ESC, 'A'};

  return buffer_t(buffer, buffer + sizeof(buffer));
}

size_t
LabelWriterDriver::getEmptyLinesCount()
{
  return emptyLinesCount;
}

void
LabelWriterDriver::setEmptyLinesCount(size_t value)
{
  emptyLinesCount = value;
}

////////////////////////////////////////////////////////////////
// LabelWriterDriver400
////////////////////////////////////////////////////////////////

LabelWriterDriver400::LabelWriterDriver400(IPrintEnvironment& environment):
  LabelWriterDriver(environment)
{
}

LabelWriterDriver400::~LabelWriterDriver400()
{
}

void
LabelWriterDriver400::startDoc()
{
  LabelWriterDriver::startDoc();
}

void
LabelWriterDriver400::endDoc()
{
  SendFormFeed();
}

void
LabelWriterDriver400::endPage()
{
  sendShortFormFeed();
}

buffer_t
LabelWriterDriver400::getShortFormFeedCommand()
{
  byte buffer[] = {ESC, 'G'};

  return buffer_t(buffer, buffer + sizeof(buffer));
}

void
LabelWriterDriver400::sendShortFormFeed()
{
  byte buffer[] = {ESC, 'G'};

  sendCommand(buffer, sizeof(buffer));
}

////////////////////////////////////////////////////////////////
// LabelWriterDriver TwinTurbo
////////////////////////////////////////////////////////////////

LabelWriterDriverTwinTurbo::LabelWriterDriverTwinTurbo(IPrintEnvironment& environment):
  LabelWriterDriver400(environment), roll(ROLL_AUTO)
{
}

LabelWriterDriverTwinTurbo::~LabelWriterDriverTwinTurbo()
{
}

void
LabelWriterDriverTwinTurbo::startDoc()
{
  LabelWriterDriver400::StartDoc();
  SendRollSelect(roll);
}

LabelWriterDriverTwinTurbo::roll_t
LabelWriterDriverTwinTurbo::getRoll()
{
  return roll;
}

void
LabelWriterDriverTwinTurbo::setRoll(LabelWriterDriverTwinTurbo::roll_t value)
{
  roll = value;
}

buffer_t
LabelWriterDriverTwinTurbo::getRollSelectCommand(roll_t value)
{
  byte buf[] = {ESC, 'q', '0'};

  switch (value)
  {
    case ROLL_LEFT:    buf[2] = '1'; break;
    case ROLL_RIGHT:   buf[2] = '2'; break;
    default:        buf[2] = '0'; break;
  }

  return buffer_t(buf, buf + sizeof(buf));
}

void
LabelWriterDriverTwinTurbo::sendRollSelect(LabelWriterDriverTwinTurbo::roll_t value)
{
  buffer_t buffer = getRollSelectCommand(value);

  sendCommand(&buffer[0], buffer.size());
}



}; // namespace
