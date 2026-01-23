#ifndef DUMMY_LANGUAGE_MONITOR_H
#define DUMMY_LANGUAGE_MONITOR_H

#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

namespace LanguageMonitor
{

class Dummy : public ILanguageMonitor
{
public:
   Dummy(IPrintEnvironment& environment) : environment(environment) {}
   virtual ~Dummy() {}

   virtual void startDoc() {}
   virtual void endDoc() {}

   virtual void startPage() {}
   virtual void endPage() {}

   virtual void processData(const buffer_t& data) {}

private:
   IPrintEnvironment& environment;
};

} // namespace LanguageMonitor

};

#endif // DUMMY_LANGUAGE_MONITOR_H
