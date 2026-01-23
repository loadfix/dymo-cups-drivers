#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{


DummyLanguageMonitor::DummyLanguageMonitor(IPrintEnvironment& Environment):
  Environment_(Environment)
{
}

DummyLanguageMonitor::~DummyLanguageMonitor()
{
}

void
DummyLanguageMonitor::StartDoc()
{
}

void
DummyLanguageMonitor::EndDoc()
{
}

void
DummyLanguageMonitor::StartPage()
{
}

void
DummyLanguageMonitor::EndPage()
{
}



void
DummyLanguageMonitor::ProcessData(const buffer_t& Data)
{
}


}; // namespace
