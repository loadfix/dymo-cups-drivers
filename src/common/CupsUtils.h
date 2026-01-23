#ifndef CUPS_UTILS_H
#define CUPS_UTILS_H

#include <cups/cups.h>
#include <cstdlib>

namespace DymoPrinterDriver
{
    class CCupsUtils
    {
    public:
        static const char* GetCupsOption(const char* name, int num_options, cups_option_t* options, const char* value = NULL);
    };
};

#endif // CUPS_UTILS_H
