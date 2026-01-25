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

void LabelManagerDriverInitializer::processCupsOptions(LabelManagerDriver& Driver, int num_options, cups_option_t* options)
{
    const char* option = CupsUtils::getCupsOption("DymoCutOptions", num_options, options, "Cut");

    Driver.setDeviceName(CupsUtils::getCupsOption("printer-make-and-model", num_options, options));

    if(strcasecmp(option, "Cut") == 0)
        Driver.setCutOption(ILabelManagerDriver::CUT_OPTION_CUT);
    else if(strcasecmp(option, "ChainMarks") == 0)
        Driver.setCutOption(ILabelManagerDriver::CUT_OPTION_CHAIN_MARKS);
    else
        fprintf(stderr, "WARNING: Unknown DymoCutOptions option value = %s\n", option);

    option = CupsUtils::getCupsOption("DymoLabelAlignment", num_options, options, "Center");

    if(strcasecmp(option, "Center") == 0)
        Driver.setAlignment(ILabelManagerDriver::ALIGN_CENTER);
    else if(strcasecmp(option, "Left") == 0)
        Driver.setAlignment(ILabelManagerDriver::ALIGN_LEFT);
    else if(strcasecmp(option, "Right") == 0)
        Driver.setAlignment(ILabelManagerDriver::ALIGN_RIGHT);
    else
        fprintf(stderr, "WARNING: Unknown DymoLabelAlignment option value = %s\n", option);

    option = CupsUtils::getCupsOption("DymoContinuousPaper", num_options, options, "0");

    if(strcasecmp(option, "0") == 0)
        Driver.setPaperType(IPrinterDriver::PAPER_TYPE_REGULAR);
    else if(strcasecmp(option, "1") == 0)
        Driver.setPaperType(IPrinterDriver::PAPER_TYPE_CONTINUOUS);
    else
        fprintf(stderr, "WARNING: Unknown DymoContinuousPaper option value = %s\n", option);

    option = CupsUtils::getCupsOption("DymoPrintChainMarksAtDocEnd", num_options, options, "0");

    if(strcasecmp(option, "0") == 0)
        Driver.setPrintChainMarksAtDocEnd(false);
    else if(strcasecmp(option, "1") == 0)
        Driver.setPrintChainMarksAtDocEnd(true);
    else
        fprintf(stderr, "WARNING: Unknown DymoPrintChainMarksAtDocEnd option value = %s\n", option);
}

void LabelManagerDriverInitializer::processPPDOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, ppd_file_t* ppd)
{
    if (!ppd)
        return;

    // Set device name from PPD
    Driver.setDeviceName(ppd->modelname);

    // Process cut options from PPD
    ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "DymoCutOptions");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Cut"))
            Driver.setCutOption(ILabelManagerDriver::CUT_OPTION_CUT);
        else if (!strcasecmp(choice->choice, "ChainMarks"))
            Driver.setCutOption(ILabelManagerDriver::CUT_OPTION_CHAIN_MARKS);
    }

    // Process label alignment from PPD
    choice = CupsUtils::findMarkedChoice(ppd, "DymoLabelAlignment");
    if (choice)
    {
        if (!strcasecmp(choice->choice, "Center"))
            Driver.setAlignment(ILabelManagerDriver::ALIGN_CENTER);
        else if (!strcasecmp(choice->choice, "Left"))
            Driver.setAlignment(ILabelManagerDriver::ALIGN_LEFT);
        else if (!strcasecmp(choice->choice, "Right"))
            Driver.setAlignment(ILabelManagerDriver::ALIGN_RIGHT);
    }

    // Process chain marks at doc end from PPD
    choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintChainMarksAtDocEnd");
    if (choice)
    {
        Driver.setPrintChainMarksAtDocEnd(atoi(choice->choice) != 0);
    }

    // Process continuous paper from PPD
    choice = CupsUtils::findMarkedChoice(ppd, "DymoContinuousPaper");
    if (choice)
    {
        if (atoi(choice->choice) != 0)
            Driver.setPaperType(IPrinterDriver::PAPER_TYPE_CONTINUOUS);
        else
            Driver.setPaperType(IPrinterDriver::PAPER_TYPE_REGULAR);
    }

    // Device-specific configuration based on model name
    if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape"))
    {
        Driver.setMaxPrintableWidth(96);
        Driver.setNormalLeader(75);
        Driver.setMinLeader(61);
        Driver.setAlignedLeader(43);
        Driver.setMinLabelLength(133);
        Driver.setSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape 128"))
    {
        Driver.setMaxPrintableWidth(128);
        Driver.setNormalLeader(75);
        Driver.setMinLeader(61);
        Driver.setAlignedLeader(43);
        Driver.setMinLabelLength(133);
        Driver.setSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 450"))
    {
        Driver.setMaxPrintableWidth(128);
        Driver.setNormalLeader(75);
        Driver.setMinLeader(55);
        Driver.setAlignedLeader(43);
        Driver.setMinLabelLength(133);
        Driver.setSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 400"))
    {
        Driver.setMaxPrintableWidth(96);
        Driver.setNormalLeader(75);
        Driver.setMinLeader(55);
        Driver.setAlignedLeader(43);
        Driver.setMinLabelLength(133);
        Driver.setSupportAutoCut(true);
    }
    else if (!strcasecmp(ppd->modelname, "DYMO LabelPOINT 350"))
    {
        Driver.setMaxPrintableWidth(96);
        Driver.setNormalLeader(75);
        Driver.setMinLeader(55);
        Driver.setAlignedLeader(43);
        Driver.setMinLabelLength(133);
        Driver.setSupportAutoCut(true);
    }
    // Add more device-specific configurations as needed
}

void LabelManagerDriverInitializer::processPageOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    ILabelManagerDriver::tape_width_t TapeWidth = ILabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff);

    fprintf(stderr, "TEST: processPageOptions device name = %s\n", Driver.getDeviceName().c_str());

    if(strcasecmp(Driver.getDeviceName().c_str(), "DYMO MobileLabeler") == 0)
    {
        if(TapeWidth == ILabelManagerDriver::TAPE_WIDTH_6MM)
            Driver.setTapeAlignmentOffset(1);
        else if (TapeWidth == ILabelManagerDriver::TAPE_WIDTH_9MM)
            Driver.setTapeAlignmentOffset(0);
        else if (TapeWidth == ILabelManagerDriver::TAPE_WIDTH_12MM)
            Driver.setTapeAlignmentOffset(2); //5
        else if (TapeWidth == ILabelManagerDriver::TAPE_WIDTH_19MM)
            Driver.setTapeAlignmentOffset(-1);
        else if (TapeWidth == ILabelManagerDriver::TAPE_WIDTH_24MM)
            Driver.setTapeAlignmentOffset(-1);
    }
}

void LabelManagerDriverInitializerWithLanguageMonitor::processCupsOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, int num_options, cups_option_t* options)
{
    LabelManagerDriverInitializer::processCupsOptions(Driver, num_options, options);

    LanguageMonitor.setDeviceName(CupsUtils::getCupsOption("printer-make-and-model", num_options, options));
}

void LabelManagerDriverInitializerWithLanguageMonitor::processPPDOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, ppd_file_t* ppd)
{
    LabelManagerDriverInitializer::processPPDOptions(Driver, LanguageMonitor, ppd);

    // Only set device name if LanguageMonitor is actually a LabelManagerLanguageMonitor
    LabelManagerLanguageMonitor* labelManagerLanguageMonitor = dynamic_cast<LabelManagerLanguageMonitor*>(&LanguageMonitor);
    if (labelManagerLanguageMonitor && ppd)
        labelManagerLanguageMonitor->setDeviceName(ppd->modelname);
}

void LabelManagerDriverInitializerWithLanguageMonitor::processPageOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    LabelManagerDriverInitializer::processPageOptions(Driver, LanguageMonitor, PageHeader);

    // Only set tape width if LanguageMonitor is actually a LabelManagerLanguageMonitor
    LabelManagerLanguageMonitor* labelManagerLanguageMonitor = dynamic_cast<LabelManagerLanguageMonitor*>(&LanguageMonitor);
    if (labelManagerLanguageMonitor)
        labelManagerLanguageMonitor->setTapeWidth(ILabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff));
}

}
