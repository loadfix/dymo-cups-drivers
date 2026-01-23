#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{


CDummyLanguageMonitor::CDummyLanguageMonitor(IPrintEnvironment& Environment):
  Environment_(Environment)
{
}

CDummyLanguageMonitor::~CDummyLanguageMonitor()
{
}

void
CDummyLanguageMonitor::StartDoc()
{
}

void
CDummyLanguageMonitor::EndDoc()
{
}

void
CDummyLanguageMonitor::StartPage()
{
}

void
CDummyLanguageMonitor::EndPage()
{
}



void
CDummyLanguageMonitor::ProcessData(const buffer_t& Data)
{
}


}; // namespace
