#include <cups/ppd.h>
#include "CupsFilterLabelManager.h"
#include "CupsUtils.h"

namespace DymoPrinterDriver
{

void
DriverInitializerLabelManager::processPPDOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, ppd_file_t* ppd)
{
  ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "DymoCutOptions");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Cut"))
      Driver.setCutOption(LabelManagerDriver::coCut);
    else if (!strcasecmp(choice->choice, "ChainMarks"))
      Driver.setCutOption(LabelManagerDriver::coChainMarks);
  }
  //else
  //    fputs("WARNING: unable to get CutOptions choice\n", stderr);


  choice = CupsUtils::findMarkedChoice(ppd, "DymoLabelAlignment");
  if (choice)
  {
    //fprintf(stderr, "DEBUG: ----------- Process LabelAlignemnt %s----------\n", choice->choice);

    if (!strcasecmp(choice->choice, "Center"))
      Driver.setAlignment(LabelManagerDriver::alCenter);
    else if (!strcasecmp(choice->choice, "Left"))
      Driver.setAlignment(LabelManagerDriver::alLeft);
    else if (!strcasecmp(choice->choice, "Right"))
      Driver.setAlignment(LabelManagerDriver::alRight);
  }
  else
    fputs("WARNING: unable to get LabelAlignment choice\n", stderr);

  choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintChainMarksAtDocEnd");
  if (choice)
  {
    Driver.setPrintChainMarksAtDocEnd((atoi(choice->choice)));
  }
  else
    fputs("WARNING: unable to get PrintChainMarksAtDocEnd choice\n", stderr);

  choice = CupsUtils::findMarkedChoice(ppd, "DymoContinuousPaper");
  if (choice)
  {
    // Note: SetContinuousPaper doesn't exist in new driver, use SetPaperType instead
    if (atoi(choice->choice) != 0)
      Driver.setPaperType(IPrinterDriver::ptContinuous);
    else
      Driver.setPaperType(IPrinterDriver::ptRegular);
  }
  else
    fputs("WARNING: unable to get ContinuousPaper choice\n", stderr);

  // Note: SetTapeColor doesn't exist in the new driver implementation
  // Tape color is not configurable in the new driver

  Driver.setDeviceName(ppd->modelname);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape"))
  {
    Driver.setMaxPrintableWidth(96);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(61);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape 128"))
  {
    Driver.setMaxPrintableWidth(128);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(61);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 450"))
  {
    Driver.setMaxPrintableWidth(128);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(55);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 400"))
  {
    Driver.setMaxPrintableWidth(96);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(55);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelPOINT 350"))
  {
    Driver.setMaxPrintableWidth(96);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(55);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PC"))
  {
    Driver.setMaxPrintableWidth(96);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(55);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PC II"))
  {
    Driver.setMaxPrintableWidth(128);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(55);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 450 DUO Tape"))
  {
    Driver.setMaxPrintableWidth(128);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(61);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(133);
    Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PnP"))
  {
    Driver.setMaxPrintableWidth(64);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(58);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(30);
    Driver.setSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelManager Wireless PnP"))
  {
     Driver.setMaxPrintableWidth(256);
     Driver.setNormalLeader(125);
     Driver.setMinLeader(92);
     Driver.setAlignedLeader(72);
     Driver.setMinLabelLength(222);
     Driver.setSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 420P"))
  {
    Driver.setMaxPrintableWidth(128);
    Driver.setNormalLeader(75);
    Driver.setMinLeader(58);
    Driver.setAlignedLeader(43);
    Driver.setMinLabelLength(63);
    Driver.setSupportAutoCut(false);
 }

 if (!strcasecmp(ppd->modelname, "DYMO LabelManager 500TS"))
 {
     Driver.setMaxPrintableWidth(256);
     Driver.setNormalLeader(125);
     Driver.setMinLeader(92);
     Driver.setAlignedLeader(72);
     Driver.setMinLabelLength(222);
     Driver.setSupportAutoCut(true);
     // Note: SetTSDevice is not available in new driver implementation
  }
}

void
DriverInitializerLabelManager::processPageOptions(LabelManagerDriver& Driver, LanguageMonitor::Dummy& LanguageMonitor, cups_page_header2_t& PageHeader)
{
  //fprintf(stderr, "DEBUG: ------ PageHeader.cupsMediaType: %d\n", PageHeader.cupsMediaType);

  // cupsMadiaType contain information about current paper
  // the lsb contain
  LabelManagerDriver::tape_width_t TapeWidth = LabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff);

  // Note: SetAutoPaper is not available in new driver implementation

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelWriter DUO Tape"))
  {
    if (TapeWidth == LabelManagerDriver::tw6mm)
      Driver.setTapeAlignmentOffset(-2);
    else if (TapeWidth == LabelManagerDriver::tw9mm)
      Driver.setTapeAlignmentOffset(-1);
  }

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelMANAGER PC II"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.setTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.setTapeAlignmentOffset(-4);
  }

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelManager 500TS"))
  {
     if (TapeWidth == LabelManagerDriver::tw12mm)
         Driver.setTapeAlignmentOffset(2);
     else if (TapeWidth == LabelManagerDriver::tw19mm)
         Driver.setTapeAlignmentOffset(-4);
 }

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelLabelWriter DUO Tape"))
  {
    if (TapeWidth == LabelManagerDriver::tw6mm)
      Driver.setTapeAlignmentOffset(-2);
    else if (TapeWidth == LabelManagerDriver::tw9mm)
      Driver.setTapeAlignmentOffset(-1);
  }

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelWriter DUO Tape 128"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.setTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.setTapeAlignmentOffset(-4);
  }

  // adjust tape center
  if (!strcasecmp(Driver.getDeviceName().c_str(), "DYMO LabelWriter 450 DUO Tape 128"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.setTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.setTapeAlignmentOffset(-4);
  }

}

void
DriverInitializerLabelManagerWithLanguageMonitor::processPPDOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
    DriverInitializerLabelManager::processPPDOptions(Driver, (LanguageMonitor::Dummy&)LanguageMonitor, ppd);

    LanguageMonitor.setDeviceName(ppd->modelname);
}

void
DriverInitializerLabelManagerWithLanguageMonitor::processPageOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    DriverInitializerLabelManager::processPageOptions(Driver, (LanguageMonitor::Dummy&)LanguageMonitor, PageHeader);

    LanguageMonitor.setTapeWidth(LabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff));
}


} // namespace
