#ifndef LABELWRITER_DRIVER_INITIALIZER_H
#define LABELWRITER_DRIVER_INITIALIZER_H

#include <string>
#include <cups/cups.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{
    class LabelWriterDriver;
    class LabelWriterLanguageMonitor;
    class DummyLanguageMonitor;
    class IPrintEnvironment;
}

namespace DymoPrinterDriver
{

bool IsLW5xxPrinter(std::string device_name);
bool IsTwinTurboPrinter(const char* model_name);
bool Is400SeriesPrinter(const char* model_name);

class LabelWriterDriverInitializer
{
public:
    static void ProcessCupsOptions(LabelWriterDriver& driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* CreateDriver(IPrintEnvironment& environment, ppd_file_t* ppd);
};

class LabelWriterDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* CreateDriver(IPrintEnvironment& environment, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
};

}

#endif // LABELWRITER_DRIVER_INITIALIZER_H
