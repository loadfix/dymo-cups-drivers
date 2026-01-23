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

void CLabelWriterDriverInitializer::ProcessCupsOptions(CLabelWriterDriver& Driver, int num_options, cups_option_t* options)
{
    const char* option = CCupsUtils::GetCupsOption("DymoPrintQuality", num_options, options, "Text");

    Driver.SetDeviceName(CCupsUtils::GetCupsOption("printer-make-and-model", num_options, options));

    if(strcasecmp(option, "Text") == 0)
        Driver.SetQuality(ILabelWriterDriver::pqText);
    else if(strcasecmp(option, "Graphics") == 0)
        Driver.SetQuality(ILabelWriterDriver::pqBarcodeAndGraphics);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintQuality option value = %s\n", option);

    option = CCupsUtils::GetCupsOption("DymoPrintDensity", num_options, options, "Normal");

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
        option = CCupsUtils::GetCupsOption("DymoPrintSpeed", num_options, options, "Normal");

        if(strcasecmp(option, "Normal") == 0)
            Driver.SetSpeed(ILabelWriterDriver::psNormal);
        else if(strcasecmp(option, "High") == 0)
            Driver.SetSpeed(ILabelWriterDriver::psHigh);
        else
            fprintf(stderr, "WARNING: Unknown DymoPrintSpeed option value = %s\n", option);

        Driver.SetSupportHighSpeed(true);
    }

    // Handle TwinTurbo roll selection
    CLabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<CLabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
        if (ppd)
        {
            ppd_choice_t* choice = CCupsUtils::FindMarkedChoice(ppd, "InputSlot");
            if (choice)
            {
                if (!strcasecmp(choice->choice, "Left"))
                    twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtLeft);
                else if (!strcasecmp(choice->choice, "Right"))
                    twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtRight);
                else
                    twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtAuto);
            }
            ppdClose(ppd);
        }
    }

    option = CCupsUtils::GetCupsOption("DymoMediaType", num_options, options, "Default");

    if(strcasecmp(option, "Default") == 0)
        Driver.SetMediaType(ILabelWriterDriver::mtDefault);
    else if(strcasecmp(option, "Durable") == 0)
        Driver.SetMediaType(ILabelWriterDriver::mtDurable);
    else
        fprintf(stderr, "WARNING: Unknown DymoMediaType option value = %s\n", option);
}

void CLabelWriterDriverInitializer::ProcessPPDOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd)
{
    if (!ppd)
        return;

    // Set device name from PPD
    Driver.SetDeviceName(ppd->modelname);

    // Process quality from PPD
    ppd_choice_t* choice = CCupsUtils::FindMarkedChoice(ppd, "DymoPrintQuality");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Text"))
            Driver.SetQuality(ILabelWriterDriver::pqText);
        else if (!strcasecmp(choice->choice, "Graphics"))
            Driver.SetQuality(ILabelWriterDriver::pqBarcodeAndGraphics);
    }

    // Process density from PPD
    choice = CCupsUtils::FindMarkedChoice(ppd, "DymoPrintDensity");
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
    CLabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<CLabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        choice = CCupsUtils::FindMarkedChoice(ppd, "InputSlot");
        if (choice)
        {
            if (!strcasecmp(choice->choice, "Left"))
                twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtLeft);
            else if (!strcasecmp(choice->choice, "Right"))
                twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtRight);
            else
                twinTurboDriver->SetRoll(CLabelWriterDriverTwinTurbo::rtAuto);
        }
    }
}

void CLabelWriterDriverInitializer::ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
    Driver.SetVerticalResolution(PageHeader.cupsHeight);
    Driver.SetHorizontalResolution(PageHeader.cupsWidth);

    if((PageHeader.cupsMediaType == int(IPrinterDriver::ptRegular)) || (PageHeader.cupsMediaType == int(IPrinterDriver::ptContinuous)))
        Driver.SetPaperType(IPrinterDriver::paper_type_t(PageHeader.cupsMediaType));
    else
        Driver.SetPaperType(IPrinterDriver::ptRegular);
}

CLabelWriterDriver* CLabelWriterDriverInitializer::CreateDriver(IPrintEnvironment& Environment, ppd_file_t* ppd)
{
    if (!ppd)
        return new CLabelWriterDriver(Environment);

    if (IsTwinTurboPrinter(ppd->modelname))
        return new CLabelWriterDriverTwinTurbo(Environment);
    else if (Is400SeriesPrinter(ppd->modelname))
        return new CLabelWriterDriver400(Environment);
    else
        return new CLabelWriterDriver(Environment);
}

void CLabelWriterDriverInitializerWithLM::ProcessCupsOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, int num_options, cups_option_t* options)
{
    CLabelWriterDriverInitializer::ProcessCupsOptions(Driver, num_options, options);

    // Handle TwinTurbo roll selection for Language Monitor
    CLabelWriterDriverTwinTurbo* twinTurboDriver = dynamic_cast<CLabelWriterDriverTwinTurbo*>(&Driver);
    if (twinTurboDriver)
    {
        // Language Monitor needs to know about roll selection
        // This would need to be implemented in LanguageMonitor if needed
    }
}

void CLabelWriterDriverInitializerWithLM::ProcessPPDOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd)
{
    CLabelWriterDriverInitializer::ProcessPPDOptions(Driver, LM, ppd);

    // Note: CLabelWriterLanguageMonitor doesn't have SetDeviceName method
    // Device name is handled through the driver
}

void CLabelWriterDriverInitializerWithLM::ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
    CLabelWriterDriverInitializer::ProcessPageOptions(Driver, LM, PageHeader);
}

CLabelWriterDriver* CLabelWriterDriverInitializerWithLM::CreateDriver(IPrintEnvironment& Environment, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
    CLabelWriterDriver* driver = CLabelWriterDriverInitializer::CreateDriver(Environment, ppd);
    // Language Monitor initialization would go here if needed
    return driver;
}

}
