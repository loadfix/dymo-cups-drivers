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

bool isLW5xxPrinter(std::string device_name);
bool isTwinTurboPrinter(const char* model_name);
bool is400SeriesPrinter(const char* model_name);

class LabelWriterDriverInitializer
{
public:
    static void processCupsOptions(LabelWriterDriver& driver, int num_options, cups_option_t* options);
    static void processPPDOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void processPageOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* createDriver(IPrintEnvironment& environment, ppd_file_t* ppd);
};

class LabelWriterDriverInitializerWithLM
{
public:
    static void processCupsOptions(LabelWriterDriver& driver, LabelWriterLanguageMonitor& language_monitor, int num_options, cups_option_t* options);
    static void processPPDOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, ppd_file_t* ppd);
    static void processPageOptions(LabelWriterDriver& driver, DummyLanguageMonitor& language_monitor, cups_page_header2_t& page_header);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* createDriver(IPrintEnvironment& environment, LabelWriterLanguageMonitor& language_monitor, ppd_file_t* ppd);
};

}

#endif // LABELWRITER_DRIVER_INITIALIZER_H
