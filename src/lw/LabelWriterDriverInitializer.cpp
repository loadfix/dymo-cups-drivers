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

bool IsLW5xxPrinter(std::string deviceName)
{

    return deviceName.rfind("DYMO LabelWriter 550", 0) == 0 || deviceName.rfind("DYMO LabelWriter 5XL", 0) == 0;
}

bool IsTwinTurboPrinter(const char* modelName)
{
    if (!modelName)
        return false;

    return !strcasecmp(modelName, "DYMO LabelWriter Twin Turbo")
        || !strcasecmp(modelName, "DYMO LabelWriter 450 Twin Turbo");
}

bool Is400SeriesPrinter(const char* modelName)
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

void LabelWriterDriverInitializer::ProcessCupsOptions(LabelWriterDriver& Driver, int num_options, cups_option_t* options)
{
    const char* option = CupsUtils::GetCupsOption("DymoPrintQuality", num_options, options, "Text");

    Driver.SetDeviceName(CupsUtils::GetCupsOption("printer-make-and-model", num_options, options));

    if(strcasecmp(option, "Text") == 0)
        Driver.SetQuality(ILabelWriterDriver::pqText);
    else if(strcasecmp(option, "Graphics") == 0)
        Driver.SetQuality(ILabelWriterDriver::pqBarcodeAndGraphics);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintQuality option value = %s\n", option);

    option = CupsUtils::GetCupsOption("DymoPrintDensity", num_options, options, "Normal");

    if(strcasecmp(option, "Light") == 0)
        Driver.SetDensity(ILabelWriterDriver::pdLow);
    else if(strcasecmp(option, "Medium") == 0)
        Driver.SetDensity(ILabelWriterDriver::pdMedium);
    else if(strcasecmp(option, "Normal") == 0)
        Driver.SetDensity(ILabelWriterDriver::pdNormal);
    else if(strcasecmp(option, "Dark") == 0)
        Driver.SetDensity(ILabelWriterDriver::pdHigh);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintDensity option value = %s\n", option);

    if(IsLW5xxPrinter(Driver.GetDeviceName()))
    {
        option = CupsUtils::GetCupsOption("DymoPrintSpeed", num_options, options, "Normal");

        if(strcasecmp(option, "Normal") == 0)
            Driver.SetSpeed(ILabelWriterDriver::psNormal);
        else if(strcasecmp(option, "High") == 0)
            Driver.SetSpeed(ILabelWriterDriver::psHigh);
        else
            fprintf(stderr, "WARNING: Unknown DymoPrintSpeed option value = %s\n", option);

        Driver.SetSupportHighSpeed(true);
    }

    // Handle TwinTurbo roll selection
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
        if (ppd)
        {
            ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "InputSlot");
            if (choice)
            {
                if (!strcasecmp(choice->choice, "Left"))
                    twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtLeft);
                else if (!strcasecmp(choice->choice, "Right"))
                    twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtRight);
                else
                    twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtAuto);
            }
            ppdClose(ppd);
        }
    }

    option = CupsUtils::GetCupsOption("DymoMediaType", num_options, options, "Default");

    if(strcasecmp(option, "Default") == 0)
        Driver.SetMediaType(ILabelWriterDriver::mtDefault);
    else if(strcasecmp(option, "Durable") == 0)
        Driver.SetMediaType(ILabelWriterDriver::mtDurable);
    else
        fprintf(stderr, "WARNING: Unknown DymoMediaType option value = %s\n", option);
}

void LabelWriterDriverInitializer::ProcessPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd)
{
    if (!ppd)
        return;

    // Set device name from PPD
    Driver.SetDeviceName(ppd->modelname);

    // Process quality from PPD
    ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintQuality");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Text"))
            Driver.SetQuality(ILabelWriterDriver::pqText);
        else if (!strcasecmp(choice->choice, "Graphics"))
            Driver.SetQuality(ILabelWriterDriver::pqBarcodeAndGraphics);
    }

    // Process density from PPD
    choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintDensity");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Light"))
            Driver.SetDensity(ILabelWriterDriver::pdLow);
        else if (!strcasecmp(choice->choice, "Medium"))
            Driver.SetDensity(ILabelWriterDriver::pdMedium);
        else if (!strcasecmp(choice->choice, "Normal"))
            Driver.SetDensity(ILabelWriterDriver::pdNormal);
        else if (!strcasecmp(choice->choice, "Dark"))
            Driver.SetDensity(ILabelWriterDriver::pdHigh);
    }

    // Set max printable width for specific models
    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 300")
     || !strcasecmp(ppd->modelname, "DYMO LabelWriter 310")
     || !strcasecmp(ppd->modelname, "DYMO LabelWriter 315"))
        Driver.SetMaxPrintableWidth(58);

    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 4XL"))
        Driver.SetMaxPrintableWidth(156);

    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter SE450"))
        Driver.SetMaxPrintableWidth(56);

    // Handle TwinTurbo roll selection
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        choice = CupsUtils::FindMarkedChoice(ppd, "InputSlot");
        if (choice)
        {
            if (!strcasecmp(choice->choice, "Left"))
                twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtLeft);
            else if (!strcasecmp(choice->choice, "Right"))
                twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtRight);
            else
                twinTurboDriver->SetRoll(LabelWriterDriverTwinTurbo::rtAuto);
        }
    }
}

void LabelWriterDriverInitializer::ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
    Driver.SetVerticalResolution(PageHeader.cupsHeight);
    Driver.SetHorizontalResolution(PageHeader.cupsWidth);

    if((PageHeader.cupsMediaType == int(IPrinterDriver::ptRegular)) || (PageHeader.cupsMediaType == int(IPrinterDriver::ptContinuous)))
        Driver.SetPaperType(IPrinterDriver::paper_type_t(PageHeader.cupsMediaType));
    else
        Driver.SetPaperType(IPrinterDriver::ptRegular);
}

LabelWriterDriver* LabelWriterDriverInitializer::CreateDriver(IPrintEnvironment& Environment, ppd_file_t* ppd)
{
    if (!ppd)
        return new LabelWriterDriver(Environment);

    if (IsTwinTurboPrinter(ppd->modelname))
        return new LabelWriterDriverTwinTurbo(Environment);
    else if (Is400SeriesPrinter(ppd->modelname))
        return new LabelWriterDriver400(Environment);
    else
        return new LabelWriterDriver(Environment);
}

void LabelWriterDriverInitializerWithLM::ProcessCupsOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, int num_options, cups_option_t* options)
{
    LabelWriterDriverInitializer::ProcessCupsOptions(Driver, num_options, options);

    // Handle TwinTurbo roll selection for Language Monitor
    LabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<LabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        // Language Monitor needs to know about roll selection
        // This would need to be implemented in LanguageMonitor if needed
    }
}

void LabelWriterDriverInitializerWithLM::ProcessPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd)
{
    LabelWriterDriverInitializer::ProcessPPDOptions(Driver, LM, ppd);

    // Note: LabelWriterLanguageMonitor doesn't have SetDeviceName method
    // Device name is handled through the driver
}

void LabelWriterDriverInitializerWithLM::ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
    LabelWriterDriverInitializer::ProcessPageOptions(Driver, LM, PageHeader);
}

LabelWriterDriver* LabelWriterDriverInitializerWithLM::CreateDriver(IPrintEnvironment& Environment, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
    LabelWriterDriver* driver = LabelWriterDriverInitializer::CreateDriver(Environment, ppd);
    // Language Monitor initialization would go here if needed
    return driver;
}

}
