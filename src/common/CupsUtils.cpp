#include "CupsUtils.h"
#include <cups/cups.h>
#include <cups/ppd.h>

namespace DymoPrinterDriver
{

const char* CupsUtils::getCupsOption(const char* name, int num_options, cups_option_t* options, const char* value)
{
    const char* option = cupsGetOption(name, num_options, options);

    if(!option)
        return value;

    return option;
}

// Replacement for deprecated ppdFindMarkedChoice
// Uses ppdFindOption and iterates through choices to find the marked one
ppd_choice_t* CupsUtils::findMarkedChoice(ppd_file_t* ppd, const char* keyword)
{
    if (!ppd || !keyword)
        return NULL;

    ppd_option_t* option = ppdFindOption(ppd, keyword);
    if (!option)
        return NULL;

    for (int i = 0; i < option->num_choices; i++)
    {
        if (option->choices[i].marked)
            return &option->choices[i];
    }

    return NULL;
}

}
