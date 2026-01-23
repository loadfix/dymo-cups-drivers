#include "LabelWriterDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelWriterDriver::LabelWriterDriver(IPrintEnvironment& Environment) :
   _printEnvironment(Environment),
   _dwVerticalResolution(0),
   _dwHorizontalResolution(0),
   _dwPageNumber(0),
   _dwJobID(0),
   _deviceName(),
   _dwHeight(0),
   _dwMaxPrintableWidth(MAX_PRINTABLE_WIDTH),
   _density(pdNormal),
   _quality(pqText),
   _speed(psNormal),
   _paperType(ptRegular),
   _mediaType(mtDefault),
   _support_high_speed(false)
{ }

void LabelWriterDriver::StartDoc()
{
   _dwPageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   _dwJobID = std::rand();
}

void LabelWriterDriver::EndDoc()
{
   SetFormFeed();
   SetEndPrintJob();
}

void LabelWriterDriver::StartPage()
{
    // Set the header for first page
    if(_dwPageNumber <= 1)
    {
        SetStartPrintJob(_dwJobID);
        SetPrintDensity();
        SetPrintQuality();
        SetPrintSpeed();
        SetPrintMedia();
    }

    if(_paperType == IPrinterDriver::ptContinuous)
        SetLabelLength(0xFFFF);
    else
        SetLabelLength(0);

    // Short form feed has been moved to EndPage() since it isn't needed here anymore
    //else
        //SetShortFormFeed();

    // Check if vertical resolution is byte aligned
    _dwHeight = 0;
    int remainder = _dwVerticalResolution % 8;
    if(remainder == 0)
        _dwHeight = _dwVerticalResolution;
    else
        _dwHeight = _dwVerticalResolution + 8 - remainder;

    // Set page number
    SetLabelIndex(_dwPageNumber);

    // Set print data header
    SetPrintDataHeader(_dwVerticalResolution, _dwHorizontalResolution);
}

void LabelWriterDriver::EndPage()
{
   _dwPageNumber++;

   // PBB is feeding only the delta (form feed - short form feed) on form feed. Therefore
   // we have to do a short form feed at the end of each page.
   SetShortFormFeed();
}

void LabelWriterDriver::ProcessRasterLine(const buffer_t& lineBuffer)
{
   buffer_t b = lineBuffer;

   // Make sure that the raster line is as long as it has been specified in the label header
   unsigned int nHeight = _dwHeight / 8;
   if(b.size() > nHeight)
      b.resize(nHeight);
   else if(b.size() < nHeight)
      b.resize(nHeight, 0);

   ProcessRasterLineInternal(b);
}

bool LabelWriterDriver::IsBlank(const buffer_t& buf)
{
    size_t size = buf.size();

    for(size_t i = 0; i < size; i++)
    {
        if(buf[i] != 0xFF) // white
            return false;
    }

    return true;
}

void LabelWriterDriver::SetStartPrintJob(const dword dwJobID)
{
   byte cmdBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwJobID >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwJobID >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwJobID >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwJobID & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetEndPrintJob()
{
   byte cmdBuffer[] = {ESC, 'Q'};

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetLabelIndex(const dword dwPageNumber)
{
   byte cmdBuffer[] = {ESC, 'n', 0x00, 0x00};

   cmdBuffer[3] = (byte)((dwPageNumber >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwPageNumber & 0xff);

   SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetPrintDataHeader(const dword dwVerticalResolution, const dword dwHorizontalResolution)
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

void LabelWriterDriver::SetFormFeed()
{
    byte cmdBuffer[] = {ESC, 'E'};

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetShortFormFeed()
{
    byte cmdBuffer[] = {ESC, 'G'};

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetPrintDensity()
{
    byte cmdBuffer[] = {ESC, 'e'};

    switch (_density)
    {
        case pdLow:
            cmdBuffer[1] = 'c';
            break;
        case pdMedium:
            cmdBuffer[1] = 'd';
            break;
        case pdNormal:
            cmdBuffer[1] = 'e';
            break;
        case pdHigh:
            cmdBuffer[1] = 'g';
            break;
        default:
            break;
    }

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetPrintQuality()
{
    byte cmdBuffer[] = {ESC, 'h'};

    switch(_quality)
    {
        case pqText:
            cmdBuffer[1] = 'h';
            break;
        case pqBarcodeAndGraphics:
            cmdBuffer[1] = 'i';
            break;
        default:
            break;
    }

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetPrintSpeed()
{
    if(!_support_high_speed)
        return;

    byte cmdBuffer[] = {ESC, 'T', 0x10};

    switch(_speed)
    {
        case psNormal:
            cmdBuffer[2] = 0x10;
            break;
        case psHigh:
            cmdBuffer[2] = 0x20;
            break;
        default:
            break;
    }

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetPrintMedia()
{
    byte cmdBuffer[] = {ESC, 'M', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    switch(_mediaType)
    {
        case mtDefault:
            cmdBuffer[2] = 0x00;
            break;
        case mtDurable:
            cmdBuffer[2] = 0x01;
            break;
        default:
            break;
    }

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::SetLabelLength(const dword dwLength)
{
    byte cmdBuffer[] = {ESC, 'L', 0, 0};

    cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
    cmdBuffer[2] = (byte)(dwLength & 0xff);

    SendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::ProcessRasterLineInternal(const buffer_t& lineBuffer)
{
    SendCommand(lineBuffer);
}

void LabelWriterDriver::SendCommand(const buffer_t& cmdBuffer)
{
    _printEnvironment.WriteData(cmdBuffer);
}

buffer_t LabelWriterDriver::GetResetCommand()
{
    return buffer_t(156, ESC);
}

buffer_t LabelWriterDriver::GetRequestStatusCommand()
{
    byte buf[] = {ESC, 'A'};
    return buffer_t(buf, buf + sizeof(buf));
}

};
