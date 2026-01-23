#include "LabelWriterDriver400.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

CLabelWriterDriver400::CLabelWriterDriver400(IPrintEnvironment& Environment) :
   CLabelWriterDriver(Environment)
{
}

void CLabelWriterDriver400::StartDoc()
{
   CLabelWriterDriver::StartDoc();
}

void CLabelWriterDriver400::EndDoc()
{
   SetFormFeed();
   // Note: 400 series doesn't send EndPrintJob command
}

void CLabelWriterDriver400::EndPage()
{
   // Increment page number (from base class)
   _dwPageNumber++;

   // 400 series uses short form feed at end of page
   SendShortFormFeed();
}

buffer_t CLabelWriterDriver400::GetShortFormFeedCommand()
{
   byte buf[] = {ESC, 'G'};

   return buffer_t(buf, buf + sizeof(buf));
}

void CLabelWriterDriver400::SendShortFormFeed()
{
   byte buf[] = {ESC, 'G'};

   SendCommand(buffer_t(buf, buf + sizeof(buf)));
}

};
