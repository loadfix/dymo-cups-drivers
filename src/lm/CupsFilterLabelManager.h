#ifndef CUPS_FILTER_LABELMANAGER_H
#define CUPS_FILTER_LABELMANAGER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include "LabelManagerDriver.h"
#include "LabelManagerLanguageMonitor.h"
#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

class DriverInitializerLabelManager
{
public:
  static void ProcessPPDOptions (LabelManagerDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class DriverInitializerLabelManagerWithLM
{
public:
  static void ProcessPPDOptions (LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // CUPS_FILTER_LABELMANAGER_H
