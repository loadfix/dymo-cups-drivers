#include "LabelWriterDriverInitializer.h"
#include <cups/ppd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "LabelWriterDriver.h"
#include "LabelWriterDriverTwinTurbo.h"
#include "LabelWriterLanguageMonitor.h"
#include "CupsUtils.h"
#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

namespace LabelWriter
{

namespace Driver
{

bool isLW5xxPrinter(std::string deviceName)
{

    return deviceName.rfind("DYMO LabelWriter 550", 0) == 0 || deviceName.rfind("DYMO LabelWriter 5XL", 0) == 0;
}

bool isTwinTurboPrinter(const char* modelName)
{
    if (!modelName)
        return false;

    return !strcasecmp(modelName, "DYMO LabelWriter Twin Turbo")
        || !strcasecmp(modelName, "DYMO LabelWriter 450 Twin Turbo");
}

bool is400SeriesPrinter(const char* modelName)
{
    if (!modelName)
        return false;

    return !strcasecmp(modelName, "DYMO LabelWriter 400")
        || !strcasecmp(modelName, "DYMO LabelWriter 400 Turbo")
        || !strcasecmp(modelName, "DYMO LabelWriter DUO Label")
        || !strcasecmp(modelName, "DYMO LabelWriter 4XL")
        || !strcasecmp(modelName, "DYMO LabelWriter 450")
        || !strcasecmp(modelName, "DYMO LabelWriter 450 Turbo")
        || !strcasecmp(modelName, "DYMO LabelWriter 450 DUO Label");
}

void Initializer::processCupsOptions(LabelWriterDriver& Driver, int num_options, cups_option_t* options)
{
    const char* option = CupsUtils::getCupsOption("DymoPrintQuality", num_options, options, "Text");

    Driver.setDeviceName(CupsUtils::getCupsOption("printer-make-and-model", num_options, options));

    if(strcasecmp(option, "Text") == 0)
        Driver.setQuality(ILabelWriterDriver::PRINT_QUALITY_TEXT);
    else if(strcasecmp(option, "Graphics") == 0)
        Driver.setQuality(ILabelWriterDriver::PRINT_QUALITY_BARCODE_AND_GRAPHICS);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintQuality option value = %s\n", option);

    option = CupsUtils::getCupsOption("DymoPrintDensity", num_options, options, "Normal");

    if(strcasecmp(option, "Light") == 0)
        Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_LOW);
    else if(strcasecmp(option, "Medium") == 0)
        Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_MEDIUM);
    else if(strcasecmp(option, "Normal") == 0)
        Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_NORMAL);
    else if(strcasecmp(option, "Dark") == 0)
        Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_HIGH);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintDensity option value = %s\n", option);

    if(isLW5xxPrinter(Driver.getDeviceName()))
    {
        option = CupsUtils::getCupsOption("DymoPrintSpeed", num_options, options, "Normal");

        if(strcasecmp(option, "Normal") == 0)
            Driver.setSpeed(ILabelWriterDriver::PRINT_SPEED_NORMAL);
        else if(strcasecmp(option, "High") == 0)
            Driver.setSpeed(ILabelWriterDriver::PRINT_SPEED_HIGH);
        else
            fprintf(stderr, "WARNING: Unknown DymoPrintSpeed option value = %s\n", option);

        Driver.setSupportHighSpeed(true);
    }

    // Handle TwinTurbo roll selection
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
        if (ppd)
        {
            ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "InputSlot");
            if (choice)
            {
                if (!strcasecmp(choice->choice, "Left"))
                    twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_LEFT);
                else if (!strcasecmp(choice->choice, "Right"))
                    twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_RIGHT);
                else
                    twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_AUTO);
            }
            ppdClose(ppd);
        }
    }

    option = CupsUtils::getCupsOption("DymoMediaType", num_options, options, "Default");

    if(strcasecmp(option, "Default") == 0)
        Driver.setMediaType(ILabelWriterDriver::MEDIA_TYPE_DEFAULT);
    else if(strcasecmp(option, "Durable") == 0)
        Driver.setMediaType(ILabelWriterDriver::MEDIA_TYPE_DURABLE);
    else
        fprintf(stderr, "WARNING: Unknown DymoMediaType option value = %s\n", option);
}

void Initializer::processPPDOptions(LabelWriterDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, ppd_file_t* ppd)
{
    if (!ppd)
        return;

    // Set device name from PPD
    Driver.setDeviceName(ppd->modelname);

    // Process quality from PPD
    ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintQuality");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Text"))
            Driver.setQuality(ILabelWriterDriver::PRINT_QUALITY_TEXT);
        else if (!strcasecmp(choice->choice, "Graphics"))
            Driver.setQuality(ILabelWriterDriver::PRINT_QUALITY_BARCODE_AND_GRAPHICS);
    }

    // Process density from PPD
    choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintDensity");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Light"))
            Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_LOW);
        else if (!strcasecmp(choice->choice, "Medium"))
            Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_MEDIUM);
        else if (!strcasecmp(choice->choice, "Normal"))
            Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_NORMAL);
        else if (!strcasecmp(choice->choice, "Dark"))
            Driver.setDensity(ILabelWriterDriver::PRINT_DENSITY_HIGH);
    }

    // Set max printable width for specific models
    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 300")
     || !strcasecmp(ppd->modelname, "DYMO LabelWriter 310")
     || !strcasecmp(ppd->modelname, "DYMO LabelWriter 315"))
        Driver.setMaxPrintableWidth(58);

    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 4XL"))
        Driver.setMaxPrintableWidth(156);

    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter SE450"))
        Driver.setMaxPrintableWidth(56);

    // Handle TwinTurbo roll selection
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        choice = CupsUtils::findMarkedChoice(ppd, "InputSlot");
        if (choice)
        {
            if (!strcasecmp(choice->choice, "Left"))
                twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_LEFT);
            else if (!strcasecmp(choice->choice, "Right"))
                twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_RIGHT);
            else
                twinTurboDriver->setRoll(LabelWriterDriverTwinTurbo::ROLL_AUTO);
        }
    }
}

void Initializer::processPageOptions(LabelWriterDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    Driver.setVerticalResolution(PageHeader.cupsHeight);
    Driver.setHorizontalResolution(PageHeader.cupsWidth);

    if((PageHeader.cupsMediaType == int(IPrinterDriver::PAPER_TYPE_REGULAR)) || (PageHeader.cupsMediaType == int(IPrinterDriver::PAPER_TYPE_CONTINUOUS)))
        Driver.setPaperType(IPrinterDriver::paper_type_t(PageHeader.cupsMediaType));
    else
        Driver.setPaperType(IPrinterDriver::PAPER_TYPE_REGULAR);
}

LabelWriterDriver* Initializer::createDriver(IPrintEnvironment& Environment, ppd_file_t* ppd)
{
    if (!ppd)
        return new LabelWriterDriver(Environment);

    if (isTwinTurboPrinter(ppd->modelname))
        return new LabelWriterDriverTwinTurbo(Environment);
    else if (is400SeriesPrinter(ppd->modelname))
        return new LabelWriterDriver400(Environment);
    else
        return new LabelWriterDriver(Environment);
}

void InitializerWithLanguageMonitor::processCupsOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LanguageMonitor, int num_options, cups_option_t* options)
{
    Initializer::processCupsOptions(Driver, num_options, options);

    // Handle TwinTurbo roll selection for Language Monitor
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        // Language Monitor needs to know about roll selection
        // This would need to be implemented in LanguageMonitor if needed
    }
}

void InitializerWithLanguageMonitor::processPPDOptions(LabelWriterDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, ppd_file_t* ppd)
{
    Initializer::processPPDOptions(Driver, LanguageMonitor, ppd);

    // Note: LabelWriterLanguageMonitor doesn't have SetDeviceName method
    // Device name is handled through the driver
}

void InitializerWithLanguageMonitor::processPageOptions(LabelWriterDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    Initializer::processPageOptions(Driver, LanguageMonitor, PageHeader);
}

LabelWriterDriver* InitializerWithLanguageMonitor::createDriver(IPrintEnvironment& Environment, LabelWriterLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
    LabelWriterDriver* driver = Initializer::createDriver(Environment, ppd);
    // Language Monitor initialization would go here if needed
    return driver;
}

} // namespace Driver

} // namespace LabelWriter

}
