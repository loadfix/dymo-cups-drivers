#ifndef LABELMANAGER_DRIVER_INITIALIZER_H
#define LABELMANAGER_DRIVER_INITIALIZER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{
    class LabelManagerDriver;
    class LabelManagerLanguageMonitor;

    namespace LanguageMonitor
    {
        class Dummy;
    }
}

namespace DymoPrinterDriver
{

class LabelManagerDriverInitializer
{
public:
    static void processCupsOptions(LabelManagerDriver& driver, int num_options, cups_option_t* options);
    static void processPPDOptions(LabelManagerDriver& driver, LanguageMonitor::Dummy& language_monitor, ppd_file_t* ppd);
    static void processPageOptions(LabelManagerDriver& driver, LanguageMonitor::Dummy& language_monitor, cups_page_header2_t& page_header);
};

class LabelManagerDriverInitializerWithLanguageMonitor
{
public:
    static void processCupsOptions(LabelManagerDriver& driver, LabelManagerLanguageMonitor& language_monitor, int num_options, cups_option_t* options);
    static void processPPDOptions(LabelManagerDriver& driver, LanguageMonitor::Dummy& language_monitor, ppd_file_t* ppd);
    static void processPageOptions(LabelManagerDriver& driver, LanguageMonitor::Dummy& language_monitor, cups_page_header2_t& page_header);
};

}

#endif // LABELMANAGER_DRIVER_INITIALIZER_H
