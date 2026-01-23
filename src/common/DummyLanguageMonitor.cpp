#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{


DummyLanguageMonitor::DummyLanguageMonitor(IPrintEnvironment& environment):
  Environment_(environment)
{
}

DummyLanguageMonitor::~DummyLanguageMonitor()
{
}

void
DummyLanguageMonitor::startDoc()
{
}

void
DummyLanguageMonitor::endDoc()
{
}

void
DummyLanguageMonitor::startPage()
{
}

void
DummyLanguageMonitor::endPage()
{
}



void
DummyLanguageMonitor::processData(const buffer_t& data)
{
}


}; // namespace
