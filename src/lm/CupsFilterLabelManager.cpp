#include <cups/ppd.h>
#include "CupsFilterLabelManager.h"
#include "CupsUtils.h"

namespace DymoPrinterDriver
{

void
DriverInitializerLabelManager::ProcessPPDOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
  ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "DymoCutOptions");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Cut"))
      Driver.SetCutOption(LabelManagerDriver::coCut);
    else if (!strcasecmp(choice->choice, "ChainMarks"))
      Driver.SetCutOption(LabelManagerDriver::coChainMarks);
  }
  //else
  //    fputs("WARNING: unable to get CutOptions choice\n", stderr);


  choice = CupsUtils::FindMarkedChoice(ppd, "DymoLabelAlignment");
  if (choice)
  {
    //fprintf(stderr, "DEBUG: ----------- Process LabelAlignemnt %s----------\n", choice->choice);

    if (!strcasecmp(choice->choice, "Center"))
      Driver.SetAlignment(LabelManagerDriver::alCenter);
    else if (!strcasecmp(choice->choice, "Left"))
      Driver.SetAlignment(LabelManagerDriver::alLeft);
    else if (!strcasecmp(choice->choice, "Right"))
      Driver.SetAlignment(LabelManagerDriver::alRight);
  }
  else
    fputs("WARNING: unable to get LabelAlignment choice\n", stderr);

  choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintChainMarksAtDocEnd");
  if (choice)
  {
    Driver.SetPrintChainMarksAtDocEnd((atoi(choice->choice)));
  }
  else
    fputs("WARNING: unable to get PrintChainMarksAtDocEnd choice\n", stderr);

  choice = CupsUtils::FindMarkedChoice(ppd, "DymoContinuousPaper");
  if (choice)
  {
    // Note: SetContinuousPaper doesn't exist in new driver, use SetPaperType instead
    if (atoi(choice->choice) != 0)
      Driver.SetPaperType(IPrinterDriver::ptContinuous);
    else
      Driver.SetPaperType(IPrinterDriver::ptRegular);
  }
  else
    fputs("WARNING: unable to get ContinuousPaper choice\n", stderr);

  // Note: SetTapeColor doesn't exist in the new driver implementation
  // Tape color is not configurable in the new driver

  Driver.SetDeviceName(ppd->modelname);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape"))
  {
    Driver.SetMaxPrintableWidth(96);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(61);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter DUO Tape 128"))
  {
    Driver.SetMaxPrintableWidth(128);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(61);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 450"))
  {
    Driver.SetMaxPrintableWidth(128);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(55);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 400"))
  {
    Driver.SetMaxPrintableWidth(96);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(55);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelPOINT 350"))
  {
    Driver.SetMaxPrintableWidth(96);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(55);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PC"))
  {
    Driver.SetMaxPrintableWidth(96);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(55);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PC II"))
  {
    Driver.SetMaxPrintableWidth(128);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(55);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 450 DUO Tape"))
  {
    Driver.SetMaxPrintableWidth(128);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(61);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(133);
    Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER PnP"))
  {
    Driver.SetMaxPrintableWidth(64);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(58);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(30);
    Driver.SetSupportAutoCut(false);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelManager Wireless PnP"))
  {
     Driver.SetMaxPrintableWidth(256);
     Driver.SetNormalLeader(125);
     Driver.SetMinLeader(92);
     Driver.SetAlignedLeader(72);
     Driver.SetMinLabelLength(222);
     Driver.SetSupportAutoCut(true);
  }

  if (!strcasecmp(ppd->modelname, "DYMO LabelMANAGER 420P"))
  {
    Driver.SetMaxPrintableWidth(128);
    Driver.SetNormalLeader(75);
    Driver.SetMinLeader(58);
    Driver.SetAlignedLeader(43);
    Driver.SetMinLabelLength(63);
    Driver.SetSupportAutoCut(false);
 }

 if (!strcasecmp(ppd->modelname, "DYMO LabelManager 500TS"))
 {
     Driver.SetMaxPrintableWidth(256);
     Driver.SetNormalLeader(125);
     Driver.SetMinLeader(92);
     Driver.SetAlignedLeader(72);
     Driver.SetMinLabelLength(222);
     Driver.SetSupportAutoCut(true);
     // Note: SetTSDevice is not available in new driver implementation
  }
}

void
DriverInitializerLabelManager::ProcessPageOptions(LabelManagerDriver& Driver, DummyLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
  //fprintf(stderr, "DEBUG: ------ PageHeader.cupsMediaType: %d\n", PageHeader.cupsMediaType);

  // cupsMadiaType contain information about current paper
  // the lsb contain
  LabelManagerDriver::tape_width_t TapeWidth = LabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff);

  // Note: SetAutoPaper is not available in new driver implementation

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelWriter DUO Tape"))
  {
    if (TapeWidth == LabelManagerDriver::tw6mm)
      Driver.SetTapeAlignmentOffset(-2);
    else if (TapeWidth == LabelManagerDriver::tw9mm)
      Driver.SetTapeAlignmentOffset(-1);
  }

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelMANAGER PC II"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.SetTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.SetTapeAlignmentOffset(-4);
  }

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelManager 500TS"))
  {
     if (TapeWidth == LabelManagerDriver::tw12mm)
         Driver.SetTapeAlignmentOffset(2);
     else if (TapeWidth == LabelManagerDriver::tw19mm)
         Driver.SetTapeAlignmentOffset(-4);
 }

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelLabelWriter DUO Tape"))
  {
    if (TapeWidth == LabelManagerDriver::tw6mm)
      Driver.SetTapeAlignmentOffset(-2);
    else if (TapeWidth == LabelManagerDriver::tw9mm)
      Driver.SetTapeAlignmentOffset(-1);
  }

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelWriter DUO Tape 128"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.SetTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.SetTapeAlignmentOffset(-4);
  }

  // adjust tape center
  if (!strcasecmp(Driver.GetDeviceName().c_str(), "DYMO LabelWriter 450 DUO Tape 128"))
  {
    if (TapeWidth == LabelManagerDriver::tw12mm)
      Driver.SetTapeAlignmentOffset(2);
    else if (TapeWidth == LabelManagerDriver::tw19mm)
      Driver.SetTapeAlignmentOffset(-4);
  }

}

void
DriverInitializerLabelManagerWithLM::ProcessPPDOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
    DriverInitializerLabelManager::ProcessPPDOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, ppd);

    LanguageMonitor.SetDeviceName(ppd->modelname);
}

void
DriverInitializerLabelManagerWithLM::ProcessPageOptions(LabelManagerDriver& Driver, LabelManagerLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
    DriverInitializerLabelManager::ProcessPageOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, PageHeader);

    LanguageMonitor.SetTapeWidth(LabelManagerDriver::tape_width_t(PageHeader.cupsMediaType & 0xff));
}


} // namespace
