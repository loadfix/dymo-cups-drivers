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
    static void ProcessCupsOptions(LabelManagerDriver& driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

class LabelManagerDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(LabelManagerDriver& driver, LabelManagerLanguageMonitor& language_monitor, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelManagerDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);
};

}

#endif // LABELMANAGER_DRIVER_INITIALIZER_H
