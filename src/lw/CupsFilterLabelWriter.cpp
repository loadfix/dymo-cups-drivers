#include "CupsFilterLabelWriter.h"
#include "CupsUtils.h"

namespace DymoPrinterDriver
{

void DriverInitializerLabelWriter::ProcessPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd)
{
  // Note: Resolution is now set via ProcessPageOptions from the page header
  // The legacy SetResolution method is not available in the new driver implementation
  // Resolution is handled through SetVerticalResolution and SetHorizontalResolution

  ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintQuality");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Text"))
      Driver.SetQuality(LabelWriterDriver::pqText);
    else if (!strcasecmp(choice->choice, "Graphics"))
      Driver.SetQuality(LabelWriterDriver::pqBarcodeAndGraphics);
  }
  else
    fputs("WARNING: unable to get PrintQuality choice\n", stderr);


  choice = CupsUtils::FindMarkedChoice(ppd, "DymoPrintDensity");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Light"))
      Driver.SetDensity(LabelWriterDriver::pdLow);
    else if (!strcasecmp(choice->choice, "Medium"))
      Driver.SetDensity(LabelWriterDriver::pdMedium);
    else if (!strcasecmp(choice->choice, "Normal"))
      Driver.SetDensity(LabelWriterDriver::pdNormal);
    else if (!strcasecmp(choice->choice, "Dark"))
      Driver.SetDensity(LabelWriterDriver::pdHigh);
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
DriverInitializerLabelWriter::ProcessPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{

  if ((PageHeader.cupsMediaType == int(LabelWriterDriver::ptRegular)) || (PageHeader.cupsMediaType == int(LabelWriterDriver::ptContinuous)))
  {
    Driver.SetPaperType(LabelWriterDriver::paper_type_t(PageHeader.cupsMediaType));
  }
  else
  {
    fprintf(stderr, "WARNING: Invalid value for cupsMediaType (%d)\n", PageHeader.cupsMediaType);
    Driver.SetPaperType(LabelWriterDriver::ptRegular);
  }
  // Note: Page height is set via SetVerticalResolution in the new driver implementation
  // Page offset is not supported in the new driver
}


void
DriverInitializerLabelWriterTwinTurbo::ProcessPPDOptions(LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LM, ppd_file_t* ppd)
{
  DriverInitializerLabelWriter::ProcessPPDOptions(Driver, LM, ppd);

  ppd_choice_t* choice = CupsUtils::FindMarkedChoice(ppd, "InputSlot");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Left"))
      Driver.SetRoll(LabelWriterDriverTwinTurbo::rtLeft);
    else if (!strcasecmp(choice->choice, "Right"))
      Driver.SetRoll(LabelWriterDriverTwinTurbo::rtRight);
    else
      Driver.SetRoll(LabelWriterDriverTwinTurbo::rtAuto);
  }
  else
    fputs("WARNING: unable to get InputSlot choice\n", stderr);
}

void
DriverInitializerLabelWriterTwinTurbo::ProcessPageOptions(LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriter::ProcessPageOptions(Driver, LM, PageHeader);
}



void DriverInitializerLabelWriterWithLM::ProcessPPDOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
  DriverInitializerLabelWriter::ProcessPPDOptions(Driver, (DummyLanguageMonitor&)LM, ppd);
}

void
DriverInitializerLabelWriterWithLM::ProcessPageOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriter::ProcessPageOptions(Driver, (DummyLanguageMonitor&)LM, PageHeader);
  LM.SetPaperType(Driver.GetPaperType());
}


void
DriverInitializerLabelWriterTwinTurboWithLM::ProcessPPDOptions(LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LM, ppd_file_t* ppd)
{
  DriverInitializerLabelWriterTwinTurbo::ProcessPPDOptions(Driver, (DummyLanguageMonitor&)LM, ppd);
  LM.SetRoll(Driver.GetRoll());
}

void
DriverInitializerLabelWriterTwinTurboWithLM::ProcessPageOptions(LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LM, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriterTwinTurbo::ProcessPageOptions(Driver, (DummyLanguageMonitor&)LM, PageHeader);
}


} // namespace
