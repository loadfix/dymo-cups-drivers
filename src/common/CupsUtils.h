#ifndef CUPS_UTILS_H
#define CUPS_UTILS_H

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cstdlib>

namespace DymoPrinterDriver
{
    class CupsUtils
    {
    public:
        static const char* getCupsOption(const char* name, int num_options, cups_option_t* options, const char* value = NULL);
        // Replacement for deprecated ppdFindMarkedChoice
        static ppd_choice_t* findMarkedChoice(ppd_file_t* ppd, const char* keyword);
    };
};

#endif // CUPS_UTILS_H
