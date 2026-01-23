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
  static void ProcessPPDOptions (LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

class DriverInitializerLabelWriterTwinTurbo
{
public:
  static void ProcessPPDOptions (LabelWriterDriverTwinTurbo& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriverTwinTurbo& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

class DriverInitializerLabelWriterWithLM
{
public:
  static void ProcessPPDOptions (LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};


class DriverInitializerLabelWriterTwinTurboWithLM
{
public:
  static void ProcessPPDOptions (LabelWriterDriverTwinTurbo& driver, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriverTwinTurbo& driver, LabelWriterLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

}

#endif // CUPS_FILTER_LABELWRITER_H
