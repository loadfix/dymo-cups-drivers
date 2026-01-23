#include "LabelManagerDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelManagerDriver::LabelManagerDriver(IPrintEnvironment& environment) :
   _printEnvironment(environment),
   _dwVerticalResolution(0),
   _dwHorizontalResolution(0),
   _dwPageNumber(0),
   _dwJobID(0),
   _jobDidStart(false),
   _cutOptions(ILabelManagerDriver::coCut),
   _alignment(ILabelManagerDriver::alCenter),
   _dwTapeAlignmentOffset(0),
   _deviceName(),
   _supportAutoCut(true),
   _printChainMarksAtDocEnd(false),
   _dwMinPageLine(MIN_LABEL_LENGTH),
   _dwHeight(0),
   _dwMaxPrintableWidth(MAX_PRINTABLE_WIDTH),
   _dwNormalLeader(NORMAL_LEADER),
   _dwMinLeader(MIN_LEADER),
   _dwAlignedLeader(MIN_ALIGNED_LEADER)
{ }

void LabelManagerDriver::startDoc()
{
   _dwPageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   _dwJobID = std::rand();
}

void LabelManagerDriver::endDoc()
{
   if(!_jobDidStart)
       return;

   if(_printChainMarksAtDocEnd)
       setCutterMark();

   // Advance to cutter
   setFormFeed();

   // Cut label at the end
   if(_supportAutoCut && !_printChainMarksAtDocEnd)
      setCutCommand();

   // set end of file
   setEndPrintJob();
}

void LabelManagerDriver::startPage()
{
   // set the header for first page
   if(_dwPageNumber <= 1)
   {
      setStartPrintJob(_dwJobID);

      _jobDidStart = true;
   }

   if(_dwPageNumber > 1)
   {
      // Advance to cutter
      setFormFeed();

      if((_cutOptions == ILabelManagerDriver::coCut) && _supportAutoCut)
         setCutCommand();
      else
         setCutterMark();
   }

   // Make sure the label is long enough
   if(_dwHorizontalResolution > _dwMinPageLine)
   {
      // Check if vertical resolution is byte aligned
      _dwHeight = 0;
      int remainder = _dwVerticalResolution % 8;
      if(remainder == 0)
         _dwHeight = _dwVerticalResolution;
      else
         _dwHeight = _dwVerticalResolution + 8 - remainder;
   }
   // TODO: else throw exception
   // Check if it is support by all platforms

   // set page number
   setLabelIndex(_dwPageNumber);

   // set leader
   setLabelLeader(_dwNormalLeader - _dwMinLeader);

   // set trailer
   setLabelTrailer(_dwAlignedLeader);

   // set print data header
   setPrintDataHeader(_dwVerticalResolution, _dwHorizontalResolution);
}

void LabelManagerDriver::endPage()
{
   _dwPageNumber++;
}

void LabelManagerDriver::processRasterLine(const buffer_t& lineBuffer)
{
   buffer_t b = lineBuffer;

   // Make sure that the raster line is as long as it has been specified in the label header
   unsigned int nHeight = _dwHeight / 8;
   if(b.size() > nHeight)
      b.resize(nHeight);
   else if(b.size() < nHeight)
      b.resize(nHeight, 0);

   // It should be always center aligned
   if(_alignment != alLeft)
      processRasterLineInternal(b);
   // We don't have to save for future reversing since MLS prints just
   // center aligned which is taken care by the FW
      //RasterLines_.push_back(b); // save for future reversing
}

void LabelManagerDriver::setStartPrintJob(const dword dwJobID)
{
   byte cmdBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwJobID >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwJobID >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwJobID >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwJobID & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setEndPrintJob()
{
   byte cmdBuffer[] = {ESC, 'Q'};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setLabelIndex(const dword dwPageNumber)
{
   byte cmdBuffer[] = {ESC, 'n', 0x00, 0x00};

   cmdBuffer[3] = (byte)((dwPageNumber >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwPageNumber & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setLabelLeader(const dword dwLength)
{
   byte cmdBuffer[] = {ESC, 'l', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwLength >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwLength >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwLength & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setLabelTrailer(const dword dwLength)
{
   byte cmdBuffer[] = {ESC, 't', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwLength >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwLength >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwLength & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setPrintDataHeader(const dword dwVerticalResolution, const dword dwHorizontalResolution)
{
   // Monochrome data and bottom aligned
   byte dataBuffer[] = {ESC, 'D', 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

   // Width
   dataBuffer[7] = (byte)((dwHorizontalResolution >> 24) & 0xff);
   dataBuffer[6] = (byte)((dwHorizontalResolution >> 16) & 0xff);
   dataBuffer[5] = (byte)((dwHorizontalResolution >> 8) & 0xff);
   dataBuffer[4] = (byte)(dwHorizontalResolution & 0xff);

   // Height
   dataBuffer[11] = (byte)((_dwHeight >> 24) & 0xff);
   dataBuffer[10] = (byte)((_dwHeight >> 16) & 0xff);
   dataBuffer[9] = (byte)((_dwHeight >> 8) & 0xff);
   dataBuffer[8] = (byte)(_dwHeight & 0xff);

   sendCommand(buffer_t(dataBuffer, dataBuffer + sizeof(dataBuffer)));
}

void LabelManagerDriver::setFormFeed()
{
   byte cmdBuffer[] = {ESC, 'E'};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setShortFormFeed()
{
   byte cmdBuffer[] = {ESC, 'G'};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setCutCommand()
{
   byte cmdBuffer[] = {ESC, 'p', 0x30};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::setCutterMark()
{
   byte cmdBuffer[] = {ESC, 'p', 0x31};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::processRasterLineInternal(const buffer_t& LineBuffer)
{
   buffer_t shiftedRasterLine = LineBuffer;

   shiftData(LineBuffer, shiftedRasterLine, getShiftValue(LineBuffer.size()));

   sendCommand(shiftedRasterLine);
}

void LabelManagerDriver::shiftData(const buffer_t& Buf, buffer_t& shiftedBuf, int shiftValue)
{
   // Clear shift buffer first
   for(size_t i = 0; i < shiftedBuf.size(); ++i)
      shiftedBuf[i] = 0;

   if(shiftValue >= 0)
      shiftDataRight(Buf, shiftedBuf, shiftValue);
   else
      shiftDataLeft(Buf, shiftedBuf, -shiftValue);
}

void LabelManagerDriver::shiftDataLeft(const buffer_t& Buf, buffer_t& shiftedBuf, size_t shiftValue)
{
   // shift bytes first
   unsigned int shiftedLen = shiftedBuf.size() - shiftValue / 8;
   shiftValue = shiftValue % 8;

   if((shiftedLen <= 0) || (Buf.size() == 0))
      return;

   // shift bits
   size_t i = 0;
   for(i = 0; ((i < Buf.size() - 1) && (i < size_t(shiftedLen))); ++i)
      shiftedBuf[i] = (Buf[i] << shiftValue) | (Buf[i + 1] >> (8 - shiftValue));
   if(i < size_t(shiftedLen))
      shiftedBuf[Buf.size() - 1] = (Buf[Buf.size() - 1] << shiftValue); // last
}

void LabelManagerDriver::shiftDataRight(const buffer_t& Buf, buffer_t& shiftedBuf, size_t shiftValue)
{
   // shift bytes first
   unsigned int shiftedLen = shiftedBuf.size() - shiftValue / 8;
   size_t shiftedOffset = shiftValue / 8;
   shiftValue = shiftValue % 8;

   if((shiftedLen <= 0) || (Buf.size() == 0))
      return;

   // shift bits
   shiftedBuf[shiftedOffset] = Buf[0] >> shiftValue; // first
   size_t i = 0;
   for(i = 1; ((i < Buf.size()) && (i < size_t(shiftedLen))); ++i)
      shiftedBuf[shiftedOffset + i] = (Buf[i - 1] << (8 - shiftValue)) | (Buf[i] >> shiftValue);
   if(i < size_t(shiftedLen))
      shiftedBuf[shiftedOffset + Buf.size()] = (Buf[Buf.size() - 1] << (8 - shiftValue));
}

int LabelManagerDriver::getShiftValue(size_t RasterLineSize)
{
   return _dwTapeAlignmentOffset;
}

void LabelManagerDriver::sendCommand(const buffer_t& cmdBuffer)
{
   _printEnvironment.writeData(cmdBuffer);
}

buffer_t LabelManagerDriver::getRequestStatusCommand()
{
    byte buf[] = {ESC, 'A'};
    return buffer_t(buf, buf + sizeof(buf));
}

};
