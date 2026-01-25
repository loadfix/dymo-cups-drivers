#include "LabelManagerDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelManagerDriver::LabelManagerDriver(IPrintEnvironment& environment) :
   printEnvironment(environment),
   verticalResolution(0),
   horizontalResolution(0),
   pageNumber(0),
   jobID(0),
   jobDidStart(false),
   cutOptions(ILabelManagerDriver::CUT_OPTION_CUT),
   alignment(ILabelManagerDriver::ALIGN_CENTER),
   tapeAlignmentOffset(0),
   deviceName(),
   supportAutoCut(true),
   printChainMarksAtDocEnd(false),
   minPageLine(MIN_LABEL_LENGTH),
   height(0),
   maxPrintableWidth(MAX_PRINTABLE_WIDTH),
   normalLeader(NORMAL_LEADER),
   minLeader(MIN_LEADER),
   alignedLeader(MIN_ALIGNED_LEADER)
{ }

void LabelManagerDriver::startDoc()
{
   pageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   jobID = std::rand();
}

void LabelManagerDriver::endDoc()
{
   if(!jobDidStart)
       return;

   if(printChainMarksAtDocEnd)
       setCutterMark();

   // Advance to cutter
   setFormFeed();

   // Cut label at the end
   if(supportAutoCut && !printChainMarksAtDocEnd)
      setCutCommand();

   // set end of file
   setEndPrintJob();
}

void LabelManagerDriver::startPage()
{
   // set the header for first page
   if(pageNumber <= 1)
   {
      setStartPrintJob(jobID);

      jobDidStart = true;
   }

   if(pageNumber > 1)
   {
      // Advance to cutter
      setFormFeed();

      if((cutOptions == ILabelManagerDriver::CUT_OPTION_CUT) && supportAutoCut)
         setCutCommand();
      else
         setCutterMark();
   }

   // Make sure the label is long enough
   if(horizontalResolution > minPageLine)
   {
      // Check if vertical resolution is byte aligned
      height = 0;
      int remainder = verticalResolution % 8;
      if(remainder == 0)
         height = verticalResolution;
      else
         height = verticalResolution + 8 - remainder;
   }
   // TODO: else throw exception
   // Check if it is support by all platforms

   // set page number
   setLabelIndex(pageNumber);

   // set leader
   setLabelLeader(normalLeader - minLeader);

   // set trailer
   setLabelTrailer(alignedLeader);

   // set print data header
   setPrintDataHeader(verticalResolution, horizontalResolution);
}

void LabelManagerDriver::endPage()
{
   pageNumber++;
}

void LabelManagerDriver::processRasterLine(const buffer_t& lineBuffer)
{
   buffer_t b = lineBuffer;

   // Make sure that the raster line is as long as it has been specified in the label header
   unsigned int nHeight = height / 8;
   if(b.size() > nHeight)
      b.resize(nHeight);
   else if(b.size() < nHeight)
      b.resize(nHeight, 0);

   // It should be always center aligned
   if(alignment != ALIGN_LEFT)
      processRasterLineInternal(b);
   // We don't have to save for future reversing since MLS prints just
   // center aligned which is taken care by the FW
      //RasterLines_.push_back(b); // save for future reversing
}

void LabelManagerDriver::setStartPrintJob(const dword jobID)
{
   byte commandBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   commandBuffer[5] = (byte)((jobID >> 24) & 0xff);
   commandBuffer[4] = (byte)((jobID >> 16) & 0xff);
   commandBuffer[3] = (byte)((jobID >> 8) & 0xff);
   commandBuffer[2] = (byte)(jobID & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setEndPrintJob()
{
   byte commandBuffer[] = {ESC, 'Q'};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setLabelIndex(const dword pageNumber)
{
   byte commandBuffer[] = {ESC, 'n', 0x00, 0x00};

   commandBuffer[3] = (byte)((pageNumber >> 8) & 0xff);
   commandBuffer[2] = (byte)(pageNumber & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setLabelLeader(const dword length)
{
   byte commandBuffer[] = {ESC, 'l', 0x00, 0x00, 0x00, 0x00};

   commandBuffer[5] = (byte)((length >> 24) & 0xff);
   commandBuffer[4] = (byte)((length >> 16) & 0xff);
   commandBuffer[3] = (byte)((length >> 8) & 0xff);
   commandBuffer[2] = (byte)(length & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setLabelTrailer(const dword length)
{
   byte commandBuffer[] = {ESC, 't', 0x00, 0x00, 0x00, 0x00};

   commandBuffer[5] = (byte)((length >> 24) & 0xff);
   commandBuffer[4] = (byte)((length >> 16) & 0xff);
   commandBuffer[3] = (byte)((length >> 8) & 0xff);
   commandBuffer[2] = (byte)(length & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setPrintDataHeader(const dword verticalResolution, const dword horizontalResolution)
{
   // Monochrome data and bottom aligned
   byte dataBuffer[] = {ESC, 'D', 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

   // Width
   dataBuffer[7] = (byte)((horizontalResolution >> 24) & 0xff);
   dataBuffer[6] = (byte)((horizontalResolution >> 16) & 0xff);
   dataBuffer[5] = (byte)((horizontalResolution >> 8) & 0xff);
   dataBuffer[4] = (byte)(horizontalResolution & 0xff);

   // Height
   dataBuffer[11] = (byte)((height >> 24) & 0xff);
   dataBuffer[10] = (byte)((height >> 16) & 0xff);
   dataBuffer[9] = (byte)((height >> 8) & 0xff);
   dataBuffer[8] = (byte)(height & 0xff);

   sendCommand(buffer_t(dataBuffer, dataBuffer + sizeof(dataBuffer)));
}

void LabelManagerDriver::setFormFeed()
{
   byte commandBuffer[] = {ESC, 'E'};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setShortFormFeed()
{
   byte commandBuffer[] = {ESC, 'G'};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setCutCommand()
{
   byte commandBuffer[] = {ESC, 'p', 0x30};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelManagerDriver::setCutterMark()
{
   byte commandBuffer[] = {ESC, 'p', 0x31};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
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
   return tapeAlignmentOffset;
}

void LabelManagerDriver::sendCommand(const buffer_t& commandBuffer)
{
   printEnvironment.writeData(commandBuffer);
}

buffer_t LabelManagerDriver::getRequestStatusCommand()
{
    byte buf[] = {ESC, 'A'};
    return buffer_t(buf, buf + sizeof(buf));
}

};
