#ifndef LABELMANAGER_DRIVER_INITIALIZER_H
#define LABELMANAGER_DRIVER_INITIALIZER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{
    class CLabelManagerDriver;
    class CLabelManagerLanguageMonitor;
    class CDummyLanguageMonitor;
}

namespace DymoPrinterDriver
{

class CLabelManagerDriverInitializer
{
public:
    static void ProcessCupsOptions(CLabelManagerDriver& Driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

class CLabelManagerDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(CLabelManagerDriver& Driver, CLabelManagerLanguageMonitor& LM, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(CLabelManagerDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);
};

}

#endif // LABELMANAGER_DRIVER_INITIALIZER_H
