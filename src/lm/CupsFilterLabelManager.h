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
  static void ProcessPPDOptions (LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

class DriverInitializerLabelManagerWithLM
{
public:
  static void ProcessPPDOptions (LabelManagerDriver& driver, LabelManagerLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelManagerDriver& driver, LabelManagerLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

}

#endif // CUPS_FILTER_LABELMANAGER_H
