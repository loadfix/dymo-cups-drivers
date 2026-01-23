#ifndef LABELWRITER_DRIVER_INITIALIZER_H
#define LABELWRITER_DRIVER_INITIALIZER_H

#include <string>
#include <cups/cups.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{
    class CLabelWriterDriver;
    class CLabelWriterLanguageMonitor;
    class CDummyLanguageMonitor;
    class IPrintEnvironment;
}

namespace DymoPrinterDriver
{

bool IsLW5xxPrinter(std::string deviceName);
bool IsTwinTurboPrinter(const char* modelName);
bool Is400SeriesPrinter(const char* modelName);

class CLabelWriterDriverInitializer
{
public:
    static void ProcessCupsOptions(CLabelWriterDriver& Driver, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);

    // Factory method to create appropriate driver based on model
    static CLabelWriterDriver* CreateDriver(IPrintEnvironment& Environment, ppd_file_t* ppd);
};

class CLabelWriterDriverInitializerWithLM
{
public:
    static void ProcessCupsOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, int num_options, cups_option_t* options);
    static void ProcessPPDOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd);
    static void ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader);

    // Factory method to create appropriate driver based on model
    static CLabelWriterDriver* CreateDriver(IPrintEnvironment& Environment, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd);
};

}

#endif // LABELWRITER_DRIVER_INITIALIZER_H
