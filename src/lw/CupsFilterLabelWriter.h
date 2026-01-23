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
  static void ProcessPPDOptions (LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class DriverInitializerLabelWriterTwinTurbo
{
public:
  static void ProcessPPDOptions (LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class DriverInitializerLabelWriterWithLM
{
public:
  static void ProcessPPDOptions (LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};


class DriverInitializerLabelWriterTwinTurboWithLM
{
public:
  static void ProcessPPDOptions (LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // CUPS_FILTER_LABELWRITER_H
