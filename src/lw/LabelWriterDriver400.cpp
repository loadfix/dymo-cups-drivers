#include "LabelWriterDriver400.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

LabelWriterDriver400::LabelWriterDriver400(IPrintEnvironment& Environment) :
   LabelWriterDriver(Environment)
{
}

void LabelWriterDriver400::StartDoc()
{
   LabelWriterDriver::StartDoc();
}

void LabelWriterDriver400::EndDoc()
{
   SetFormFeed();
   // Note: 400 series doesn't send EndPrintJob command
}

void LabelWriterDriver400::EndPage()
{
   // Increment page number (from base class)
   _dwPageNumber++;

   // 400 series uses short form feed at end of page
   SendShortFormFeed();
}

buffer_t LabelWriterDriver400::GetShortFormFeedCommand()
{
   byte buf[] = {ESC, 'G'};

   return buffer_t(buf, buf + sizeof(buf));
}

void LabelWriterDriver400::SendShortFormFeed()
{
   byte buf[] = {ESC, 'G'};

   SendCommand(buffer_t(buf, buf + sizeof(buf)));
}

};
