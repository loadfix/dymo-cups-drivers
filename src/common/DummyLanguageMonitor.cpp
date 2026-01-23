#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

namespace LanguageMonitor
{


Dummy::Dummy(IPrintEnvironment& environment):
  environment(environment)
{
}

Dummy::~Dummy()
{
}

void
Dummy::startDoc()
{
}

void
Dummy::endDoc()
{
}

void
Dummy::startPage()
{
}

void
Dummy::endPage()
{
}



void
Dummy::processData(const buffer_t& data)
{
}

} // namespace LanguageMonitor

}; // namespace
