#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <memory>
#include <string.h>
#include <csignal>
#include "LabelManagerDriver.h"
#include "LabelManagerLanguageMonitor.h"
#include "DummyLanguageMonitor.h"
#include "CupsPrintEnvironment.h"
#include "CupsFilter.h"
#include "CupsFilterLabelManager.h"
#include "LabelManagerDriverInitializer.h"
#include <cups/cups.h>
#include <cups/raster.h>
#include <stdlib.h>

using namespace DymoPrinterDriver;

// Global filter pointer for signal handling
template<class Driver, class DriverInitializer, class LanguageMonitor>
CupsFilter<Driver, DriverInitializer, LanguageMonitor>* gFilterPtr = nullptr;

static bool isBackchannelSupported()
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
    fputs("DEBUG: starting (raster2dymolm)\n", stderr);

    if (isBackchannelSupported())
    {
        // Note: LanguageMonitor support to be added later
        // For now using DummyLanguageMonitor until LabelManagerLanguageMonitor is copied
        return runFilter<LabelManagerDriver, LabelManagerDriverInitializerWithLM, DummyLanguageMonitor>(argc, argv);
    }
    else
    {
        return runFilter<LabelManagerDriver, LabelManagerDriverInitializer, DummyLanguageMonitor>(argc, argv);
    }
}
