#include "LabelWriterDriver400.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

LabelWriterDriver400::LabelWriterDriver400(IPrintEnvironment& environment) :
   LabelWriterDriver(environment)
{
}

void LabelWriterDriver400::startDoc()
{
   LabelWriterDriver::startDoc();
}

void LabelWriterDriver400::endDoc()
{
   setFormFeed();
   // Note: 400 series doesn't send EndPrintJob command
}

void LabelWriterDriver400::endPage()
{
   // Increment page number (from base class)
   pageNumber++;

   // 400 series uses short form feed at end of page
   sendShortFormFeed();
}

buffer_t LabelWriterDriver400::getShortFormFeedCommand()
{
   byte buf[] = {ESC, 'G'};

   return buffer_t(buf, buf + sizeof(buf));
}

void LabelWriterDriver400::sendShortFormFeed()
{
   byte buf[] = {ESC, 'G'};

   sendCommand(buffer_t(buf, buf + sizeof(buf)));
}

};
