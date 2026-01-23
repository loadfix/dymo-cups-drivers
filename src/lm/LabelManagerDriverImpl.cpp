#include "LabelManagerDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelManagerDriver::LabelManagerDriver(IPrintEnvironment& Environment) :
   _printEnvironment(Environment),
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

void LabelManagerDriver::StartDoc()
{
   _dwPageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   _dwJobID = std::rand();
}

void LabelManagerDriver::EndDoc()
{
   if(!_jobDidStart)
       return;

   if(_printChainMarksAtDocEnd)
       SetCutterMark();

   // Advance to cutter
   SetFormFeed();

   // Cut label at the end
   if(_supportAutoCut && !_printChainMarksAtDocEnd)
      SetCutCommand();

   // Set end of file
   SetEndPrintJob();
}

void LabelManagerDriver::StartPage()
{
   // Set the header for first page
   if(_dwPageNumber <= 1)
   {
      SetStartPrintJob(_dwJobID);

      _jobDidStart = true;
   }

   if(_dwPageNumber > 1)
   {
      // Advance to cutter
      SetFormFeed();

      if((_cutOptions == ILabelManagerDriver::coCut) && _supportAutoCut)
         SetCutCommand();
      else
         SetCutterMark();
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

   // Set page number
   SetLabelIndex(_dwPageNumber);

   // Set leader
   SetLabelLeader(_dwNormalLeader - _dwMinLeader);

   // Set trailer
   SetLabelTrailer(_dwAlignedLeader);

   // Set print data header
   SetPrintDataHeader(_dwVerticalResolution, _dwHorizontalResolution);
}

void LabelManagerDriver::EndPage()
{
   _dwPageNumber++;
}

void LabelManagerDriver::ProcessRasterLine(const buffer_t& lineBuffer)
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
      ProcessRasterLineInternal(b);
   // We don't have to save for future reversing since MLS prints just
   // center aligned which is taken care by the FW
      //RasterLines_.push_back(b); // save for future reversing
}

void LabelManagerDriver::SetStartPrintJob(const dword dwJobID)
{
   byte cmdBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwJobID >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwJobID >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwJobID >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwJobID & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetEndPrintJob()
{
   byte cmdBuffer[] = {ESC, 'Q'};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetLabelIndex(const dword dwPageNumber)
{
   byte cmdBuffer[] = {ESC, 'n', 0x00, 0x00};

   cmdBuffer[3] = (byte)((dwPageNumber >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwPageNumber & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetLabelLeader(const dword dwLength)
{
   byte cmdBuffer[] = {ESC, 'l', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwLength >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwLength >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwLength & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetLabelTrailer(const dword dwLength)
{
   byte cmdBuffer[] = {ESC, 't', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwLength >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwLength >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwLength & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetPrintDataHeader(const dword dwVerticalResolution, const dword dwHorizontalResolution)
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

   SendCommand(buffer_t(dataBuffer, dataBuffer + sizeof(dataBuffer)));
}

void LabelManagerDriver::SetFormFeed()
{
   byte cmdBuffer[] = {ESC, 'E'};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetShortFormFeed()
{
   byte cmdBuffer[] = {ESC, 'G'};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetCutCommand()
{
   byte cmdBuffer[] = {ESC, 'p', 0x30};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::SetCutterMark()
{
   byte cmdBuffer[] = {ESC, 'p', 0x31};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelManagerDriver::ProcessRasterLineInternal(const buffer_t& LineBuffer)
{
   buffer_t ShiftedRasterLine = LineBuffer;

   ShiftData(LineBuffer, ShiftedRasterLine, GetShiftValue(LineBuffer.size()));

   SendCommand(ShiftedRasterLine);
}

void LabelManagerDriver::ShiftData(const buffer_t& Buf, buffer_t& ShiftedBuf, int ShiftValue)
{
   // Clear shift buffer first
   for(size_t i = 0; i < ShiftedBuf.size(); ++i)
      ShiftedBuf[i] = 0;

   if(ShiftValue >= 0)
      ShiftDataRight(Buf, ShiftedBuf, ShiftValue);
   else
      ShiftDataLeft(Buf, ShiftedBuf, -ShiftValue);
}

void LabelManagerDriver::ShiftDataLeft(const buffer_t& Buf, buffer_t& ShiftedBuf, size_t ShiftValue)
{
   // Shift bytes first
   unsigned int ShiftedLen = ShiftedBuf.size() - ShiftValue / 8;
   ShiftValue = ShiftValue % 8;

   if((ShiftedLen <= 0) || (Buf.size() == 0))
      return;

   // Shift bits
   size_t i = 0;
   for(i = 0; ((i < Buf.size() - 1) && (i < size_t(ShiftedLen))); ++i)
      ShiftedBuf[i] = (Buf[i] << ShiftValue) | (Buf[i + 1] >> (8 - ShiftValue));
   if(i < size_t(ShiftedLen))
      ShiftedBuf[Buf.size() - 1] = (Buf[Buf.size() - 1] << ShiftValue); // last
}

void LabelManagerDriver::ShiftDataRight(const buffer_t& Buf, buffer_t& ShiftedBuf, size_t ShiftValue)
{
   // Shift bytes first
   unsigned int ShiftedLen = ShiftedBuf.size() - ShiftValue / 8;
   size_t ShiftedOffset = ShiftValue / 8;
   ShiftValue = ShiftValue % 8;

   if((ShiftedLen <= 0) || (Buf.size() == 0))
      return;

   // Shift bits
   ShiftedBuf[ShiftedOffset] = Buf[0] >> ShiftValue; // first
   size_t i = 0;
   for(i = 1; ((i < Buf.size()) && (i < size_t(ShiftedLen))); ++i)
      ShiftedBuf[ShiftedOffset + i] = (Buf[i - 1] << (8 - ShiftValue)) | (Buf[i] >> ShiftValue);
   if(i < size_t(ShiftedLen))
      ShiftedBuf[ShiftedOffset + Buf.size()] = (Buf[Buf.size() - 1] << (8 - ShiftValue));
}

int LabelManagerDriver::GetShiftValue(size_t RasterLineSize)
{
   return _dwTapeAlignmentOffset;
}

void LabelManagerDriver::SendCommand(const buffer_t& cmdBuffer)
{
   _printEnvironment.WriteData(cmdBuffer);
}

buffer_t LabelManagerDriver::GetRequestStatusCommand()
{
    byte buf[] = {ESC, 'A'};
    return buffer_t(buf, buf + sizeof(buf));
}

};
