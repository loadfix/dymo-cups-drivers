#include "CupsFilterLabelWriter.h"

namespace DymoPrinterDriver
{

void CDriverInitializerLabelWriter::ProcessPPDOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd)
{
  // Note: Resolution is now set via ProcessPageOptions from the page header
  // The legacy SetResolution method is not available in the new driver implementation
  // Resolution is handled through SetVerticalResolution and SetHorizontalResolution

  ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "DymoPrintQuality");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Text"))
      Driver.SetQuality(CLabelWriterDriver::pqText);
    else if (!strcasecmp(choice->choice, "Graphics"))
      Driver.SetQuality(CLabelWriterDriver::pqBarcodeAndGraphics);
  }
  else
    fputs("WARNING: unable to get PrintQuality choice\n", stderr);


  choice = ppdFindMarkedChoice(ppd, "DymoPrintDensity");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Light"))
      Driver.SetDensity(CLabelWriterDriver::pdLow);
    else if (!strcasecmp(choice->choice, "Medium"))
      Driver.SetDensity(CLabelWriterDriver::pdMedium);
    else if (!strcasecmp(choice->choice, "Normal"))
      Driver.SetDensity(CLabelWriterDriver::pdNormal);
    else if (!strcasecmp(choice->choice, "Dark"))
      Driver.SetDensity(CLabelWriterDriver::pdHigh);
  }
  else
    fputs("WARNING: unable to get PrintDensity choice\n", stderr);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 300")
  || !strcasecmp(ppd->modelname, "DYMO LabelWriter 310")
  || !strcasecmp(ppd->modelname, "DYMO LabelWriter 315"))
    Driver.SetMaxPrintableWidth(58);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 4XL"))
    Driver.SetMaxPrintableWidth(156);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter SE450"))
    Driver.SetMaxPrintableWidth(56);
}

void
CDriverInitializerLabelWriter::ProcessPageOptions(CLabelWriterDriver& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{

  if ((PageHeader.cupsMediaType == int(CLabelWriterDriver::ptRegular)) || (PageHeader.cupsMediaType == int(CLabelWriterDriver::ptContinuous)))
  {
    Driver.SetPaperType(CLabelWriterDriver::paper_type_t(PageHeader.cupsMediaType));
  }
  else
  {
    fprintf(stderr, "WARNING: Invalid value for cupsMediaType (%d)\n", PageHeader.cupsMediaType);
    Driver.SetPaperType(CLabelWriterDriver::ptRegular);
  }
  // Note: Page height is set via SetVerticalResolution in the new driver implementation
  // Page offset is not supported in the new driver
}


void
CDriverInitializerLabelWriterTwinTurbo::ProcessPPDOptions(CLabelWriterDriverTwinTurbo& Driver, CDummyLanguageMonitor& LM, ppd_file_t* ppd)
{
  CDriverInitializerLabelWriter::ProcessPPDOptions(Driver, LM, ppd);

  ppd_choice_t* choice = ppdFindMarkedChoice(ppd, "InputSlot");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Left"))
      Driver.SetRoll(CLabelWriterDriverTwinTurbo::rtLeft);
    else if (!strcasecmp(choice->choice, "Right"))
      Driver.SetRoll(CLabelWriterDriverTwinTurbo::rtRight);
    else
      Driver.SetRoll(CLabelWriterDriverTwinTurbo::rtAuto);
  }
  else
    fputs("WARNING: unable to get InputSlot choice\n", stderr);
}

void
CDriverInitializerLabelWriterTwinTurbo::ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, CDummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  CDriverInitializerLabelWriter::ProcessPageOptions(Driver, LM, PageHeader);
}



void CDriverInitializerLabelWriterWithLM::ProcessPPDOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
  CDriverInitializerLabelWriter::ProcessPPDOptions(Driver, (CDummyLanguageMonitor&)LM, ppd);
}

void
CDriverInitializerLabelWriterWithLM::ProcessPageOptions(CLabelWriterDriver& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  CDriverInitializerLabelWriter::ProcessPageOptions(Driver, (CDummyLanguageMonitor&)LM, PageHeader);
  LM.SetPaperType(Driver.GetPaperType());
}


void
CDriverInitializerLabelWriterTwinTurboWithLM::ProcessPPDOptions(CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
  CDriverInitializerLabelWriterTwinTurbo::ProcessPPDOptions(Driver, (CDummyLanguageMonitor&)LM, ppd);
  LM.SetRoll(Driver.GetRoll());
}

void
CDriverInitializerLabelWriterTwinTurboWithLM::ProcessPageOptions(CLabelWriterDriverTwinTurbo& Driver, CLabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  CDriverInitializerLabelWriterTwinTurbo::ProcessPageOptions(Driver, (CDummyLanguageMonitor&)LM, PageHeader);
}


} // namespace
