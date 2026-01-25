namespace DymoPrinterDriver
{


const byte ESC = 0x1B;
const byte SYN = 0x16;

LabelManagerDriver::LabelManagerDriver(IPrintEnvironment& environment):
  environment(environment),
  cutOptions(LabelManagerDriver::CUT_OPTION_CUT), alignment(ALIGN_CENTER), continuousPaper(false), printChainMarksAtDocEnd(false), autoPaper(false), tapeAlignmentOffset(0), tapeColor(TAPE_COLOR_BLACK_ON_WHITE),
  deviceName(), supportAutoCut(true), tsDevice(false), maxPrintableWidth(96),
  normalLeader(75), minLeader(55), alignedLeader(43), minPageLines(133),
  lastDotTab(size_t(-1)), lastBytesPerLine(size_t(-1)), emptyLinesCount(0), pageNo(1),
  rasterLines(), shiftedRasterLine(12), tsBuffer(0),
  hLockFile(0)
{
}

LabelManagerDriver::~LabelManagerDriver()
{
}

void
LabelManagerDriver::startDoc()
{
  pageNo = 1;

  sendCommand(buffer_t(getMaxBytesPerLine(), 0)); // clean

  sendTapeColor(tapeColor);
}

void
LabelManagerDriver::endDoc()
{
  if (printChainMarksAtDocEnd)
    sendChainMark();

  sendSkipLines(minLeader); // advance to the cutter

  if (supportAutoCut && !printChainMarksAtDocEnd)
    sendCut();

  if(isTSDevice())
  {
      flushCommandTS();
      endCommandTS();
  }
}

void
LabelManagerDriver::startPage()
{
  lastDotTab         = size_t(-1);
  lastBytesPerLine   = size_t(-1);
  pageLineCount      = 0;
  emptyLinesCount    = 0;

  rasterLines.clear();

  size_t leaderLength = normalLeader;
  if (pageNo > 1)
  {
    if ((cutOptions == CUT_OPTION_CUT) && supportAutoCut)
    {
      sendSkipLines(minLeader); // advance to the cutter
      leaderLength -= minLeader;
      sendCut();
    }
    else
      sendChainMark();
  }
  else // first page
    leaderLength -= minLeader; // already at cutter position

  if (!continuousPaper)
    sendSkipLines(leaderLength);
}

void
LabelManagerDriver::endPage()
{
  // last empty lines will not be drawn in case of Auto paper
  // so, adjust the page length to properly calculate min page length
  if (autoPaper)
    pageLineCount -= emptyLinesCount;

  // process cached data
  if (alignment == ALIGN_LEFT)
  {
    size_t minLabelLength = minPageLines + (normalLeader - alignedLeader);
    if (pageLineCount < minLabelLength)
      sendSkipLines(minLabelLength - pageLineCount);
    sendCachedRasterLines();
  }

  if (!continuousPaper)
  {
    size_t trailerLength = normalLeader;

    if (alignment != ALIGN_CENTER)
      trailerLength = alignedLeader;

    // for left we already take short label length into account
    if (alignment != ALIGN_LEFT)
    {
      size_t minLabelLength = minPageLines + (normalLeader - trailerLength);
      if (pageLineCount < minLabelLength)
        trailerLength += minLabelLength - pageLineCount;
    }

    // draw empty lines at the and, so the label has a full length
    if (!autoPaper)
      trailerLength += emptyLinesCount;
    emptyLinesCount = 0;

    SendSkipLines(trailerLength);
  }

  ++pageNo;
}


void
LabelManagerDriver::getBlanks(
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


void
LabelManagerDriver::ProcessRasterLine(const buffer_t& linebuffer)
{
  ++pageLineCount;

  buffer_t b = linebuffer;

  if (b.size() > GetMaxBytesPerLine())
    b = buffer_t(b.begin(), b.begin() + GetMaxBytesPerLine());

  if (alignment == ALIGN_LEFT)
    rasterLines.push_back(b); // save for future reversing
  else
    ProcessRasterLineInternal(b);
}

void
LabelManagerDriver::processRasterLineInternal(const buffer_t& lineBuffer)
{
  shiftData(lineBuffer, shiftedRasterLine, getShiftValue(lineBuffer.size()));

  size_t leaderBlanks = 0;
  size_t trailerBlanks = 0;

  // get blanks count
  getBlanks(shiftedRasterLine, leaderBlanks, trailerBlanks);

  if (leaderBlanks + trailerBlanks == shiftedRasterLine.size())
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
    if (lastDotTab != leaderBlanks)
    {
      sendDotTab(leaderBlanks);
      lastDotTab = leaderBlanks;
    }

    size_t bytesPerLine = shiftedRasterLine.size() - leaderBlanks - trailerBlanks;
    if (lastBytesPerLine != bytesPerLine)
    {
      lastBytesPerLine = bytesPerLine;
      sendBytesPerLine(lastBytesPerLine);
    }

    byte syn = SYN;
    sendCommand(&syn, sizeof(syn));
    sendCommand(&shiftedRasterLine[0] + leaderBlanks, bytesPerLine);
  }
}

static byte
ReverseByte(byte value)
{
  byte   ReversedByte   = 0;
  size_t BitsCopied     = 0;

  while (value)
  {
    ReversedByte <<= 1;
    if (value & 0x1)
      ReversedByte |= 0x1;

    value >>= 1;
    ++BitsCopied;
  }

  ReversedByte <<= 8 - BitsCopied;

  return ReversedByte;
}

void
LabelManagerDriver::sendCachedRasterLines()
{
  for (std::vector<buffer_t>::reverse_iterator it = rasterLines.rbegin(); it < rasterLines.rend(); ++it)
  {
    buffer_t& b = *it;
    for (size_t i = 0; i < b.size(); ++i)
      b[i] = ReverseByte(b[i]);

    std::reverse(b.begin(), b.end());

    ProcessRasterLineInternal(b);
  }
}

void
LabelManagerDriver::setMaxPrintableWidth(size_t value)
{
  maxPrintableWidth = value;
  shiftedRasterLine.resize(GetMaxBytesPerLine());
}

void
LabelManagerDriver::setNormalLeader(size_t value)
{
  normalLeader = value;
}

void
LabelManagerDriver::setMinLeader(size_t value)
{
  minLeader = value;
}

void
LabelManagerDriver::setAlignedLeader(size_t value)
{
  alignedLeader = value;
}

void
LabelManagerDriver::setMinPageLines(size_t value)
{
  minPageLines = value;
}

void
LabelManagerDriver::setCutOptions(LabelManagerDriver::cut_option_t value)
{
  cutOptions = value;
}

void
LabelManagerDriver::setAlignment(LabelManagerDriver::alignment_t value)
{
  alignment = value;
}

void
LabelManagerDriver::setContinuousPaper(bool value)
{
  continuousPaper = value;
}

void
LabelManagerDriver::setPrintChainMarksAtDocEnd(bool value)
{
  printChainMarksAtDocEnd = value;
}

void
LabelManagerDriver::setAutoPaper(bool value)
{
  autoPaper = value;
}

void
LabelManagerDriver::setTapeAlignmentOffset(int value)
{
  tapeAlignmentOffset = value;
}

void
LabelManagerDriver::setTapeColor(tape_color_t value)
{
  tapeColor = value;
}

void
LabelManagerDriver::setDeviceName(const std::string& value)
{
  deviceName = value;
}

void
LabelManagerDriver::setSupportAutoCut(bool value)
{
  supportAutoCut = value;
}

void
LabelManagerDriver::setTSDevice(bool value)
{
  tsDevice = value;
}

size_t
LabelManagerDriver::getMaxBytesPerLine()
{
  return maxPrintableWidth / 8;
}

const std::string&
LabelManagerDriver::getDeviceName()
{
  return deviceName;
}

bool
LabelManagerDriver::issupportAutoCut()
{
  return supportAutoCut;
}

bool
LabelManagerDriver::isTSDevice()
{
  return tsDevice;
}

LabelManagerDriver::cut_option_t
LabelManagerDriver::getcutOptions()
{
  return cutOptions;
}

LabelManagerDriver::alignment_t
LabelManagerDriver::getalignment()
{
  return alignment;
}

bool
LabelManagerDriver::iscontinuousPaper()
{
  return continuousPaper;
}

bool
LabelManagerDriver::isprintChainMarksAtDocEnd()
{
  return printChainMarksAtDocEnd;
}

bool
LabelManagerDriver::isautoPaper()
{
  return autoPaper;
}

LabelManagerDriver::tape_color_t
LabelManagerDriver::gettapeColor()
{
  return tapeColor;
}

int
LabelManagerDriver::getTapealignmentOffset()
{
  return tapeAlignmentOffset;
}

size_t
LabelManagerDriver::getMaxPrintableWidth()
{
  return maxPrintableWidth;
}

size_t
LabelManagerDriver::getNormalLeader()
{
  return normalLeader;
}

size_t
LabelManagerDriver::getMinLeader()
{
  return minLeader;
}

size_t
LabelManagerDriver::getAlignedLeader()
{
  return alignedLeader;
}

size_t
LabelManagerDriver::getMinPageLines()
{
  return minPageLines;
}

void
LabelManagerDriver::sendCommand(const byte* buffer, size_t bufferSize)
{
  SendCommand(buffer_t(buffer, buffer + bufferSize));
}

void
LabelManagerDriver::sendCommand(const buffer_t& buffer)
{
  if(IsTSDevice())
    SendCommandTS(buffer);
  else
    environment.WriteData(buffer);
}

void
LabelManagerDriver::sendCommandTS(const buffer_t& buffer)
{
    tsBuffer.insert(tsBuffer.end(), buffer.begin(), buffer.end());

    fprintf(stderr, "DEBUG: SendCommandTS() size %d\n", int(tsBuffer.size()));

    if(tsBuffer.size() > 4096)
        flushCommandTS();
}

void
LabelManagerDriver::flushCommandTS()
{
  if(tsBuffer.size() > 0)
  {
    fprintf(stderr, "DEBUG: FlushCommandTS() size %d\n", int(tsBuffer.size()));

    byte buffer[] = {ESC, 'Y', 1, 0, 0, 0, 0};
    size_t size = tsBuffer.size();

    buffer[3] = size >> 24;
    buffer[4] = (size >> 16) & 0xFF;
    buffer[5] = (size >> 8) & 0xFF;
    buffer[6] = size & 0xFF;

    fprintf(stderr, "DEBUG: FlushCommandTS() size 0x%02X 0x%02X 0x%02X 0x%02X\n", buffer[3], buffer[4], buffer[5], buffer[6]);

    buffer_t prefix = buffer_t(buffer, buffer + 7);

    tsBuffer.insert(tsBuffer.begin(), prefix.begin(), prefix.end());

    environment.WriteData(tsBuffer);

    tsBuffer.clear();
  }
}

void
LabelManagerDriver::endCommandTS()
{
    byte buffer[] = {ESC, 'Y', 0, 0, 0, 0, 0};

    environment.WriteData(buffer_t(buffer, buffer + sizeof(buffer)));
}

void
LabelManagerDriver::sendDotTab(size_t value)
{
  byte buffer[] = {ESC, 'B', 0};
  buffer[2] = value;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelManagerDriver::sendCut()
{
  byte buffer[] = {ESC, 'E'};

  sendCommand(buffer, sizeof(buffer));
}

void
LabelManagerDriver::sendChainMark()
{
  byte buffer[] = {ESC, 'B', 0, ESC, 'D', 0, SYN};
  buffer[5] = GetMaxBytesPerLine();
  sendCommand(buffer, sizeof(buffer));

  buffer_t buffer2(GetMaxBytesPerLine(), 0x99);
  SendCommand(&buffer2[0], buffer2.size());

  lastDotTab         = size_t(-1);
  lastBytesPerLine   = size_t(-1);
}

void
LabelManagerDriver::sendBytesPerLine(size_t value)
{
  byte buffer[] = {ESC, 'D', 0};
  buffer[2] = value;

  sendCommand(buffer, sizeof(buffer));
}

void
LabelManagerDriver::sendSkipLines(size_t value)
{
  if (value > 0)
  {
    SendBytesPerLine(0);

    buffer_t buffer(value, SYN);
    SendCommand(&buffer[0], value);

    lastBytesPerLine = size_t(-1);
  }
}

void
LabelManagerDriver::sendTapeColor(tape_color_t value)
{
  byte buffer[] = {ESC, 'C', 0};
  buffer[2] = int(value);

  sendCommand(buffer, sizeof(buffer));
}

void
LabelManagerDriver::shiftDataRight(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue)
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

void
LabelManagerDriver::shiftDataLeft(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue)
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


void
LabelManagerDriver::ShiftData(const buffer_t& buffer, buffer_t& shiftedBuffer, int shiftValue)
{
  // clear shift buffer first
  for (size_t i = 0; i < shiftedBuffer.size(); ++i)
    shiftedBuffer[i] = 0;

  if (shiftValue >= 0)
    shiftDataRight(buffer, shiftedBuffer, shiftValue);
  else
    shiftDataLeft(buffer, shiftedBuffer, -shiftValue);
}

int
LabelManagerDriver::getShiftValue(size_t rasterLineSize)
{
  return (maxPrintableWidth - rasterLineSize * 8) / 2 + tapeAlignmentOffset;
}

buffer_t
LabelManagerDriver::getRequestStatusCommand()
{
    byte buffer[] = {ESC, 'A'};

    return buffer_t(buffer, buffer + sizeof(buffer));
}

}; // namespace
