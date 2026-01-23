#include "CupsUtils.h"
#include <cups/cups.h>

namespace DymoPrinterDriver
{

const char* CCupsUtils::GetCupsOption(const char* name, int num_options, cups_option_t* options, const char* value)
{
    const char* option = cupsGetOption(name, num_options, options);

    if(!option)
        return value;

    return option;
}

}
