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

bool IsLW5xxPrinter(std::string deviceName);
bool IsTwinTurboPrinter(const char* modelName);
bool Is400SeriesPrinter(const char* modelName);

class LabelWriterDriverInitializer
{
public:
    static void ProcessCupsOptions(LabelWriterDriver& Driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* CreateDriver(IPrintEnvironment& Environment, ppd_file_t* ppd);
};

class LabelWriterDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);

    // Factory method to create appropriate driver based on model
    static LabelWriterDriver* CreateDriver(IPrintEnvironment& Environment, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
};

}

#endif // LABELWRITER_DRIVER_INITIALIZER_H
