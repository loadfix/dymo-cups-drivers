#ifndef CUPS_FILTER_LABELWRITER_H
#define CUPS_FILTER_LABELWRITER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>
#include "LabelWriterDriver.h"
#include "LabelWriterDriverImpl.h"
#include "LabelWriterDriverTwinTurbo.h"
#include "LabelWriterLanguageMonitor.h"
#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

class DriverInitializerLabelWriter
{
public:
  static void processPPDOptions (LabelWriterDriver& driver, LanguageMonitor::Dummy& language_monitor, ppd_file_t* ppd);
  static void processPageOptions(LabelWriterDriver& driver, LanguageMonitor::Dummy& language_monitor, cups_page_header2_t& page_header);
};

class DriverInitializerLabelWriterTwinTurbo
{
public:
  static void processPPDOptions (LabelWriterDriverTwinTurbo& driver, LanguageMonitor::Dummy& language_monitor, ppd_file_t* ppd);
  static void processPageOptions(LabelWriterDriverTwinTurbo& driver, LanguageMonitor::Dummy& language_monitor, cups_page_header2_t& page_header);
};

class DriverInitializerLabelWriterWithLanguageMonitor
{
public:
  static void processPPDOptions (LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void processPageOptions(LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};


class DriverInitializerLabelWriterTwinTurboWithLanguageMonitor
{
public:
  static void processPPDOptions (LabelWriterDriverTwinTurbo& driver, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void processPageOptions(LabelWriterDriverTwinTurbo& driver, LabelWriterLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

}

#endif // CUPS_FILTER_LABELWRITER_H
