#include "LabelWriterDriverImpl.h"
#include "PrinterDriver.h"
#include <ctime>
#include <cstdlib>

namespace DymoPrinterDriver
{

LabelWriterDriver::LabelWriterDriver(IPrintEnvironment& environment) :
   printEnvironment(environment),
   verticalResolution(0),
   horizontalResolution(0),
   pageNumber(0),
   jobID(0),
   deviceName(),
   height(0),
   maxPrintableWidth(MAX_PRINTABLE_WIDTH),
   density(pdNormal),
   quality(pqText),
   speed(psNormal),
   paperType(ptRegular),
   mediaType(mtDefault),
   supportHighSpeed(false)
{ }

void LabelWriterDriver::startDoc()
{
   pageNumber = 1;

   // Generate random number for the job ID
   std::srand(static_cast<unsigned int>(std::time(0))); // use current time as seed for random generator
   jobID = std::rand();
}

void LabelWriterDriver::endDoc()
{
   setFormFeed();
   setEndPrintJob();
}

void LabelWriterDriver::startPage()
{
    // set the header for first page
    if(pageNumber <= 1)
    {
        setStartPrintJob(jobID);
        setPrintDensity();
        setPrintQuality();
        setPrintSpeed();
        setPrintMedia();
    }

    if(paperType == IPrinterDriver::ptContinuous)
        setLabelLength(0xFFFF);
    else
        setLabelLength(0);

    // Short form feed has been moved to EndPage() since it isn't needed here anymore
    //else
        //setShortFormFeed();

    // Check if vertical resolution is byte aligned
    height = 0;
    int remainder = verticalResolution % 8;
    if(remainder == 0)
        height = verticalResolution;
    else
        height = verticalResolution + 8 - remainder;

    // set page number
    setLabelIndex(pageNumber);

    // set print data header
    setPrintDataHeader(verticalResolution, horizontalResolution);
}

void LabelWriterDriver::endPage()
{
   pageNumber++;

   // PBB is feeding only the delta (form feed - short form feed) on form feed. Therefore
   // we have to do a short form feed at the end of each page.
   setShortFormFeed();
}

void LabelWriterDriver::processRasterLine(const buffer_t& lineBuffer)
{
   buffer_t b = lineBuffer;

   // Make sure that the raster line is as long as it has been specified in the label header
   unsigned int nHeight = height / 8;
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

void LabelWriterDriver::setStartPrintJob(const dword jobID)
{
   byte commandBuffer[] = {ESC, 's', 0x00, 0x00, 0x00, 0x00};

   commandBuffer[5] = (byte)((jobID >> 24) & 0xff);
   commandBuffer[4] = (byte)((jobID >> 16) & 0xff);
   commandBuffer[3] = (byte)((jobID >> 8) & 0xff);
   commandBuffer[2] = (byte)(jobID & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setEndPrintJob()
{
   byte commandBuffer[] = {ESC, 'Q'};

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setLabelIndex(const dword pageNumber)
{
   byte commandBuffer[] = {ESC, 'n', 0x00, 0x00};

   commandBuffer[3] = (byte)((pageNumber >> 8) & 0xff);
   commandBuffer[2] = (byte)(pageNumber & 0xff);

   sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setPrintDataHeader(const dword verticalResolution, const dword horizontalResolution)
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

void LabelWriterDriver::setFormFeed()
{
    byte commandBuffer[] = {ESC, 'E'};

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setShortFormFeed()
{
    byte commandBuffer[] = {ESC, 'G'};

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setPrintDensity()
{
    byte commandBuffer[] = {ESC, 'e'};

    switch (density)
    {
        case pdLow:
            commandBuffer[1] = 'c';
            break;
        case pdMedium:
            commandBuffer[1] = 'd';
            break;
        case pdNormal:
            commandBuffer[1] = 'e';
            break;
        case pdHigh:
            commandBuffer[1] = 'g';
            break;
        default:
            break;
    }

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setPrintQuality()
{
    byte commandBuffer[] = {ESC, 'h'};

    switch(quality)
    {
        case pqText:
            commandBuffer[1] = 'h';
            break;
        case pqBarcodeAndGraphics:
            commandBuffer[1] = 'i';
            break;
        default:
            break;
    }

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setPrintSpeed()
{
    if(!supportHighSpeed)
        return;

    byte commandBuffer[] = {ESC, 'T', 0x10};

    switch(speed)
    {
        case psNormal:
            commandBuffer[2] = 0x10;
            break;
        case psHigh:
            commandBuffer[2] = 0x20;
            break;
        default:
            break;
    }

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setPrintMedia()
{
    byte commandBuffer[] = {ESC, 'M', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    switch(mediaType)
    {
        case mtDefault:
            commandBuffer[2] = 0x00;
            break;
        case mtDurable:
            commandBuffer[2] = 0x01;
            break;
        default:
            break;
    }

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::setLabelLength(const dword length)
{
    byte commandBuffer[] = {ESC, 'L', 0, 0};

    commandBuffer[3] = (byte)((length >> 8) & 0xff);
    commandBuffer[2] = (byte)(length & 0xff);

    sendCommand(buffer_t(commandBuffer, commandBuffer + sizeof(commandBuffer)));
}

void LabelWriterDriver::processRasterLineInternal(const buffer_t& lineBuffer)
{
    sendCommand(lineBuffer);
}

void LabelWriterDriver::sendCommand(const buffer_t& commandBuffer)
{
    printEnvironment.writeData(commandBuffer);
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
