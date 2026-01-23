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

class CDriverInitializerLabelWriter
{
public:
  static void ProcessPPDOptions (CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CDriverInitializerLabelWriterTwinTurbo
{
public:
  static void ProcessPPDOptions (CLabelWriterDriverTwinTurbo& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CDriverInitializerLabelWriterWithLM
{
public:
  static void ProcessPPDOptions (CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};


class CDriverInitializerLabelWriterTwinTurboWithLM
{
public:
  static void ProcessPPDOptions (CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
  static void ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // CUPS_FILTER_LABELWRITER_H
