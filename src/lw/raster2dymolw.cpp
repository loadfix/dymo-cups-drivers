#include <cups/cups.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "LabelWriterDriver.h"
#include "LabelWriterDriverImpl.h"
#include "LabelWriterDriver400.h"
#include "LabelWriterDriverTwinTurbo.h"
#include "LabelWriterLanguageMonitor.h"
#include "DummyLanguageMonitor.h"
#include "CupsPrintEnvironment.h"
#include "CupsFilter.h"
#include "CupsFilterLabelWriter.h"
#include "LabelWriterDriverInitializer.h"
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <csignal>
#include <cups/ppd.h>
#include "../common/CupsFilter.h"
#include "../common/DummyLanguageMonitor.h"
#include "LabelWriterDriver.h"
#include "LabelWriterDriverImpl.h"
#include "LabelWriterDriver400.h"
#include "LabelWriterDriverTwinTurbo.h"
#include "LabelWriterDriverInitializer.h"

using namespace DymoPrinterDriver;
using DymoPrinterDriver::LabelWriter::Driver::isTwinTurboPrinter;
using DymoPrinterDriver::LabelWriter::Driver::is400SeriesPrinter;

// Global filter pointer for signal handling
template<class Driver, class DriverInitializer, class LanguageMonitor>
CupsFilter<Driver, DriverInitializer, LanguageMonitor>* gFilterPtr = nullptr;

static bool isBackChannelSupported()
{
    return true;
}

template<class Driver, class DriverInitializer, class LanguageMonitor>
int runFilter(int argc, char* argv[])
{
    CupsFilter<Driver, DriverInitializer, LanguageMonitor> filter;
    gFilterPtr<Driver, DriverInitializer, LanguageMonitor> = &filter;

    // Filters and backends may also receive SIGPIPE when an upstream or downstream filter/backend exits
    // with a non-zero status. Developers should generally ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    auto signal_handler = [](int sig_num) {
        // make sure to unlock synchronization mutex in case process is abnormally terminated
        fprintf(stderr, "Received signal %d, aborting\n", sig_num);
        // Note: Abort() method may not be available in old CupsFilter, so we'll handle gracefully
    };

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGHUP);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGILL);
    sigaddset(&sa.sa_mask, SIGABRT);
    sigaddset(&sa.sa_mask, SIGSEGV);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGTSTP);

    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);

    int result = filter.run(argc, argv);
    gFilterPtr<Driver, DriverInitializer, LanguageMonitor> = nullptr;
    return result;
}

int main(int argc, char* argv[])
{
    fputs("DEBUG: starting (raster2dymolw)\n", stderr);

    ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
    if (!ppd)
    {
        perror("WARNING: Unable to open ppd file, use default settings - ");

        if (isBackChannelSupported())
        {
            // Note: LanguageMonitor support to be added later
            return runFilter<LabelWriterDriver, LabelWriter::Driver::InitializerWithLanguageMonitor, LanguageMonitor::Dummy>(argc, argv);
        }
        else
        {
            return runFilter<LabelWriterDriver, LabelWriter::Driver::Initializer, LanguageMonitor::Dummy>(argc, argv);
        }
    }
    else
    {
        if (isTwinTurboPrinter(ppd->modelname))
        {
            if (isBackChannelSupported())
            {
                // Note: LanguageMonitor support to be added later
                return runFilter<LabelWriterDriverTwinTurbo, LabelWriter::Driver::InitializerWithLanguageMonitor, LanguageMonitor::Dummy>(argc, argv);
            }
            else
            {
                return runFilter<LabelWriterDriverTwinTurbo, LabelWriter::Driver::Initializer, LanguageMonitor::Dummy>(argc, argv);
            }
        }
        else if (is400SeriesPrinter(ppd->modelname))
        {
            if (isBackChannelSupported())
            {
                // Note: LanguageMonitor support to be added later
                return runFilter<LabelWriterDriver400, LabelWriter::Driver::InitializerWithLanguageMonitor, LanguageMonitor::Dummy>(argc, argv);
            }
            else
            {
                return runFilter<LabelWriterDriver400, LabelWriter::Driver::Initializer, LanguageMonitor::Dummy>(argc, argv);
            }
        }
        else
        {
            if (isBackChannelSupported())
            {
                // Note: LanguageMonitor support to be added later
                return runFilter<LabelWriterDriver, LabelWriter::Driver::InitializerWithLanguageMonitor, LanguageMonitor::Dummy>(argc, argv);
            }
            else
            {
                return runFilter<LabelWriterDriver, LabelWriter::Driver::Initializer, LanguageMonitor::Dummy>(argc, argv);
            }
        }

        ppdClose(ppd);
    }
}
