#include "LabelWriterDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelWriterDriver::LabelWriterDriver(IPrintEnvironment& environment) :
   _printEnvironment(environment),
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

void LabelWriterDriver::startDoc()
{
   _dwPageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   _dwJobID = std::rand();
}

void LabelWriterDriver::endDoc()
{
   setFormFeed();
   setEndPrintJob();
}

void LabelWriterDriver::startPage()
{
    // set the header for first page
    if(_dwPageNumber <= 1)
    {
        setStartPrintJob(_dwJobID);
        setPrintDensity();
        setPrintQuality();
        setPrintSpeed();
        setPrintMedia();
    }

    if(_paperType == IPrinterDriver::ptContinuous)
        setLabelLength(0xFFFF);
    else
        setLabelLength(0);

    // Short form feed has been moved to EndPage() since it isn't needed here anymore
    //else
        //setShortFormFeed();

    // Check if vertical resolution is byte aligned
    _dwHeight = 0;
    int remainder = _dwVerticalResolution % 8;
    if(remainder == 0)
        _dwHeight = _dwVerticalResolution;
    else
        _dwHeight = _dwVerticalResolution + 8 - remainder;

    // set page number
    setLabelIndex(_dwPageNumber);

    // set print data header
    setPrintDataHeader(_dwVerticalResolution, _dwHorizontalResolution);
}

void LabelWriterDriver::endPage()
{
   _dwPageNumber++;

   // PBB is feeding only the delta (form feed - short form feed) on form feed. Therefore
   // we have to do a short form feed at the end of each page.
   setShortFormFeed();
}

void LabelWriterDriver::processRasterLine(const buffer_t& lineBuffer)
{
   buffer_t b = lineBuffer;

   // Make sure that the raster line is as long as it has been specified in the label header
   unsigned int nHeight = _dwHeight / 8;
   if(b.size() > nHeight)
      b.resize(nHeight);
   else if(b.size() < nHeight)
      b.resize(nHeight, 0);

   processRasterLineInternal(b);
}

bool LabelWriterDriver::isBlank(const buffer_t& buf)
{
    size_t size = buf.size();

    for(size_t i = 0; i < size; i++)
    {
        if(buf[i] != 0xFF) // white
            return false;
    }

    return true;
}

void LabelWriterDriver::setStartPrintJob(const dword dwJobID)
{
   byte cmdBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   cmdBuffer[5] = (byte)((dwJobID >> 24) & 0xff);
   cmdBuffer[4] = (byte)((dwJobID >> 16) & 0xff);
   cmdBuffer[3] = (byte)((dwJobID >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwJobID & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setEndPrintJob()
{
   byte cmdBuffer[] = {ESC, 'Q'};

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setLabelIndex(const dword dwPageNumber)
{
   byte cmdBuffer[] = {ESC, 'n', 0x00, 0x00};

   cmdBuffer[3] = (byte)((dwPageNumber >> 8) & 0xff);
   cmdBuffer[2] = (byte)(dwPageNumber & 0xff);

   sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setPrintDataHeader(const dword dwVerticalResolution, const dword dwHorizontalResolution)
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

void LabelWriterDriver::setFormFeed()
{
    byte cmdBuffer[] = {ESC, 'E'};

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setShortFormFeed()
{
    byte cmdBuffer[] = {ESC, 'G'};

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setPrintDensity()
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

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setPrintQuality()
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

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setPrintSpeed()
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

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setPrintMedia()
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

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::setLabelLength(const dword dwLength)
{
    byte cmdBuffer[] = {ESC, 'L', 0, 0};

    cmdBuffer[3] = (byte)((dwLength >> 8) & 0xff);
    cmdBuffer[2] = (byte)(dwLength & 0xff);

    sendCommand(buffer_t(cmdBuffer, cmdBuffer + sizeof(cmdBuffer)));
}

void LabelWriterDriver::processRasterLineInternal(const buffer_t& lineBuffer)
{
    sendCommand(lineBuffer);
}

void LabelWriterDriver::sendCommand(const buffer_t& cmdBuffer)
{
    _printEnvironment.writeData(cmdBuffer);
}

buffer_t LabelWriterDriver::getResetCommand()
{
    return buffer_t(156, ESC);
}

buffer_t LabelWriterDriver::getRequestStatusCommand()
{
    byte buf[] = {ESC, 'A'};
    return buffer_t(buf, buf + sizeof(buf));
}

};
