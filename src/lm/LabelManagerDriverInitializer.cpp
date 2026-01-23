#include "LabelManagerDriverInitializer.h"
#include <cups/ppd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "LabelManagerDriver.h"
#include "LabelManagerDriverImpl.h"
#include "LabelManagerLanguageMonitor.h"
#include "CupsUtils.h"
#include "DummyLanguageMonitor.h"

namespace DymoPrinterDriver
{

void LabelManagerDriverInitializer::ProcessCupsOptions(LabelManagerDriver& Driver, int num_options, cups_option_t* options)
{
    const char* option = CupsUtils::GetCupsOption("DymoCutOptions", num_options, options, "Cut");

    Driver.SetDeviceName(CupsUtils::GetCupsOption("printer-make-and-model", num_options, options));

    if(strcasecmp(option, "Cut") == 0)
        Driver.SetCutOption(ILabelManagerDriver::coCut);
    else if(strcasecmp(option, "ChainMarks") == 0)
        Driver.SetCutOption(ILabelManagerDriver::coChainMarks);
    else
        fprintf(stderr, "WARNING: Unknown DymoCutOptions option value = %s\n", option);

    option = CupsUtils::GetCupsOption("DymoLabelAlignment", num_options, options, "Center");

    if(strcasecmp(option, "Center") == 0)
        Driver.SetAlignment(ILabelManagerDriver::alCenter);
    else if(strcasecmp(option, "Left") == 0)
        Driver.SetAlignment(ILabelManagerDriver::alLeft);
    else if(strcasecmp(option, "Right") == 0)
        Driver.SetAlignment(ILabelManagerDriver::alRight);
    else
        fprintf(stderr, "WARNING: Unknown DymoLabelAlignment option value = %s\n", option);

    option = CupsUtils::GetCupsOption("DymoContinuousPaper", num_options, options, "0");

    if(strcasecmp(option, "0") == 0)
        Driver.SetPaperType(IPrinterDriver::ptRegular);
    else if(strcasecmp(option, "1") == 0)
        Driver.SetPaperType(IPrinterDriver::ptContinuous);
    else
        fprintf(stderr, "WARNING: Unknown DymoContinuousPaper option value = %s\n", option);

    option = CupsUtils::GetCupsOption("DymoPrintChainMarksAtDocEnd", num_options, options, "0");

    if(strcasecmp(option, "0") == 0)
        Driver.SetPrintChainMarksAtDocEnd(false);
    else if(strcasecmp(option, "1") == 0)
        Driver.SetPrintChainMarksAtDocEnd(true);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintChainMarksAtDocEnd option value = %s\n", option);
}

void LabelManagerDriverInitializer::ProcessPPDOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
    if (!ppd)
        return;

    // Set device name from PPD
    Driver.SetDeviceName(ppd->modelname);

    // Process cut options from PPD
    ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "DymoCutOptions");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Cut"))
            Driver.SetCutOption(ILabelManagerDriver::coCut);
        else if (!strcasecmp(choice->choice, "ChainMarks"))
            Driver.SetCutOption(ILabelManagerDriver::coChainMarks);
    }

    // Process label alignment from PPD
    choice = CupsUtils::FindMarkedChoice(ppd, "DymoLabelAlignment");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Center"))
            Driver.SetAlignment(ILabelManagerDriver::alCenter);
        else if (!strcasecmp(choice->choice, "Left"))
            Driver.SetAlignment(ILabelManagerDriver::alLeft);
        else if (!strcasecmp(choice->choice, "Right"))
            Driver.SetAlignment(ILabelManagerDriver::alRight);
    }

    // Process chain marks at doc end from PPD
    choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintChainMarksAtDocEnd");
    if (choice)
    {
        Driver.SetPrintChainMarksAtDocEnd(atoi(choice->choice) != 0);
    }

    // Process continuous paper from PPD
    choice = CupsUtils::FindMarkedChoice(ppd, "DymoContinuousPaper");
    if (choice)
    {
        if (atoi(choice->choice) != 0)
            Driver.SetPaperType(IPrinterDriver::ptContinuous);
        else
            Driver.SetPaperType(IPrinterDriver::ptRegular);
    }

    // Device-specific configuration based on model name
    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape"))
    {
        Driver.SetMaxPrintableWidth(96);
        Driver.SetNormalLeader(75);
        Driver.SetMinLeader(61);
        Driver.SetAlignedLeader(43);
        Driver.SetMinLabelLength(133);
        Driver.SetSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape 128"))
    {
        Driver.SetMaxPrintableWidth(128);
        Driver.SetNormalLeader(75);
        Driver.SetMinLeader(61);
        Driver.SetAlignedLeader(43);
        Driver.SetMinLabelLength(133);
        Driver.SetSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 450"))
    {
        Driver.SetMaxPrintableWidth(128);
        Driver.SetNormalLeader(75);
        Driver.SetMinLeader(55);
        Driver.SetAlignedLeader(43);
        Driver.SetMinLabelLength(133);
        Driver.SetSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 400"))
    {
        Driver.SetMaxPrintableWidth(96);
        Driver.SetNormalLeader(75);
        Driver.SetMinLeader(55);
        Driver.SetAlignedLeader(43);
        Driver.SetMinLabelLength(133);
        Driver.SetSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelPOINT 350"))
    {
        Driver.SetMaxPrintableWidth(96);
        Driver.SetNormalLeader(75);
        Driver.SetMinLeader(55);
        Driver.SetAlignedLeader(43);
        Driver.SetMinLabelLength(133);
        Driver.SetSupportAutoCut(true);
    }
    // Add more device-specific configurations as needed
}

void LabelManagerDriverInitializer::ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    ILabelManagerDriver::tape_width_t TapeWidth = ILabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff);

    fprintf(stderr, "TEST: ProcessPageOptions device name = %s\n", Driver.GetDeviceName().c_str());

    if(strcasecmp(Driver.GetDeviceName().c_str(), "DYMO MobileLabeler") == 0)
    {
        if(TapeWidth == ILabelManagerDriver::tw6mm)
            Driver.SetTapeAlignmentOffset(1);
        else if (TapeWidth == ILabelManagerDriver::tw9mm)
            Driver.SetTapeAlignmentOffset(0);
        else if (TapeWidth == ILabelManagerDriver::tw12mm)
            Driver.SetTapeAlignmentOffset(2); //5
        else if (TapeWidth == ILabelManagerDriver::tw19mm)
            Driver.SetTapeAlignmentOffset(-1);
        else if (TapeWidth == ILabelManagerDriver::tw24mm)
            Driver.SetTapeAlignmentOffset(-1);
    }
}

void LabelManagerDriverInitializerWithLM::ProcessCupsOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, int num_options, cups_option_t* options)
{
    LabelManagerDriverInitializer::ProcessCupsOptions(Driver, num_options, options);

    LanguageMonitor.SetDeviceName(CupsUtils::GetCupsOption("printer-make-and-model", num_options, options));
}

void LabelManagerDriverInitializerWithLM::ProcessPPDOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
    LabelManagerDriverInitializer::ProcessPPDOptions(Driver, LanguageMonitor, ppd);

    // Only set device name if LanguageMonitor is actually a LabelManagerLanguageMonitor
    LabelManagerLanguageMonitor* labelManagerLanguageMonitor = dynamic_cast<LabelManagerLanguageMonitor*>(&LanguageMonitor);
    if (labelManagerLanguageMonitor && ppd)
        labelManagerLanguageMonitor->SetDeviceName(ppd->modelname);
}

void LabelManagerDriverInitializerWithLM::ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    LabelManagerDriverInitializer::ProcessPageOptions(Driver, LanguageMonitor, PageHeader);

    // Only set tape width if LanguageMonitor is actually a LabelManagerLanguageMonitor
    LabelManagerLanguageMonitor* labelManagerLanguageMonitor = dynamic_cast<LabelManagerLanguageMonitor*>(&LanguageMonitor);
    if (labelManagerLanguageMonitor)
        labelManagerLanguageMonitor->SetTapeWidth(ILabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff));
}

}
