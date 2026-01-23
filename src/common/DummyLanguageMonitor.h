#ifndef DUMMY_LANGUAGE_MONITOR_H
#define DUMMY_LANGUAGE_MONITOR_H

#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

class DummyLanguageMonitor : public ILanguageMonitor
{
public:
   DummyLanguageMonitor(IPrintEnvironment& Environment) : _environment(Environment) {}
   virtual ~DummyLanguageMonitor() {}

   virtual void StartDoc() {}
   virtual void EndDoc() {}

   virtual void StartPage() {}
   virtual void EndPage() {}

   virtual void ProcessData(const buffer_t& Data) {}

private:
   IPrintEnvironment& _environment;
};

};

#endif // DUMMY_LANGUAGE_MONITOR_H
