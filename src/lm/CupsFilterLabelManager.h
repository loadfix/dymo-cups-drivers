#ifndef CUPS_FILTER_LABELMANAGER_H
#define CUPS_FILTER_LABELMANAGER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include "LabelManagerDriver.h"
#include "LabelManagerLanguageMonitor.h"
#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

class CDriverInitializerLabelManager
{
public:
  static void ProcessPPDOptions (CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CDriverInitializerLabelManagerWithLM
{
public:
  static void ProcessPPDOptions (CLabelManagerDriver& Driver, CLabelManagerLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelManagerDriver& Driver, CLabelManagerLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // CUPS_FILTER_LABELMANAGER_H
