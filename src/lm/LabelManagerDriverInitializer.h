#ifndef LABELMANAGER_DRIVER_INITIALIZER_H
#define LABELMANAGER_DRIVER_INITIALIZER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{
    class LabelManagerDriver;
    class LabelManagerLanguageMonitor;
    class DummyLanguageMonitor;
}

namespace DymoPrinterDriver
{

class LabelManagerDriverInitializer
{
public:
    static void ProcessCupsOptions(LabelManagerDriver& Driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class LabelManagerDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LM, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // LABELMANAGER_DRIVER_INITIALIZER_H
