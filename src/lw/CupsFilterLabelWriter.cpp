#include "CupsFilterLabelWriter.h"
#include "CupsUtils.h"

namespace DymoPrinterDriver
{

void DriverInitializerLabelWriter::processPPDOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
  // Note: Resolution is now set via processPageOptions from the page header
  // The legacy SetResolution method is not available in the new driver implementation
  // Resolution is handled through SetVerticalResolution and SetHorizontalResolution

  ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintQuality");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Text"))
      Driver.setQuality(LabelWriterDriver::pqText);
    else if (!strcasecmp(choice->choice, "Graphics"))
      Driver.setQuality(LabelWriterDriver::pqBarcodeAndGraphics);
  }
  else
    fputs("WARNING: unable to get PrintQuality choice\n", stderr);


  choice = CupsUtils::findMarkedChoice(ppd, "DymoPrintDensity");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Light"))
      Driver.setDensity(LabelWriterDriver::pdLow);
    else if (!strcasecmp(choice->choice, "Medium"))
      Driver.setDensity(LabelWriterDriver::pdMedium);
    else if (!strcasecmp(choice->choice, "Normal"))
      Driver.setDensity(LabelWriterDriver::pdNormal);
    else if (!strcasecmp(choice->choice, "Dark"))
      Driver.setDensity(LabelWriterDriver::pdHigh);
  }
  else
    fputs("WARNING: unable to get PrintDensity choice\n", stderr);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 300")
  || !strcasecmp(ppd->modelname, "DYMO LabelWriter 310")
  || !strcasecmp(ppd->modelname, "DYMO LabelWriter 315"))
    Driver.setMaxPrintableWidth(58);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter 4XL"))
    Driver.setMaxPrintableWidth(156);

  if (!strcasecmp(ppd->modelname, "DYMO LabelWriter SE450"))
    Driver.setMaxPrintableWidth(56);
}

void
DriverInitializerLabelWriter::processPageOptions(LabelWriterDriver& Driver, DummyLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{

  if ((PageHeader.cupsMediaType == int(LabelWriterDriver::ptRegular)) || (PageHeader.cupsMediaType == int(LabelWriterDriver::ptContinuous)))
  {
    Driver.setPaperType(LabelWriterDriver::paper_type_t(PageHeader.cupsMediaType));
  }
  else
  {
    fprintf(stderr, "WARNING: Invalid value for cupsMediaType (%d)\n", PageHeader.cupsMediaType);
    Driver.setPaperType(LabelWriterDriver::ptRegular);
  }
  // Note: Page height is set via SetVerticalResolution in the new driver implementation
  // Page offset is not supported in the new driver
}


void
DriverInitializerLabelWriterTwinTurbo::processPPDOptions(LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
  DriverInitializerLabelWriter::processPPDOptions(Driver, LanguageMonitor, ppd);

  ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "InputSlot");
  if (choice)
  {
    if (!strcasecmp(choice->choice, "Left"))
      Driver.setRoll(LabelWriterDriverTwinTurbo::rtLeft);
    else if (!strcasecmp(choice->choice, "Right"))
      Driver.setRoll(LabelWriterDriverTwinTurbo::rtRight);
    else
      Driver.setRoll(LabelWriterDriverTwinTurbo::rtAuto);
  }
  else
    fputs("WARNING: unable to get InputSlot choice\n", stderr);
}

void
DriverInitializerLabelWriterTwinTurbo::processPageOptions(LabelWriterDriverTwinTurbo& Driver, DummyLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriter::processPageOptions(Driver, LanguageMonitor, PageHeader);
}



void DriverInitializerLabelWriterWithLM::processPPDOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
  DriverInitializerLabelWriter::processPPDOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, ppd);
}

void
DriverInitializerLabelWriterWithLM::processPageOptions(LabelWriterDriver& Driver, LabelWriterLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriter::processPageOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, PageHeader);
  LanguageMonitor.setPaperType(Driver.getPaperType());
}


void
DriverInitializerLabelWriterTwinTurboWithLM::processPPDOptions(LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LanguageMonitor, ppd_file_t* ppd)
{
  DriverInitializerLabelWriterTwinTurbo::processPPDOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, ppd);
  LanguageMonitor.setRoll(Driver.getRoll());
}

void
DriverInitializerLabelWriterTwinTurboWithLM::processPageOptions(LabelWriterDriverTwinTurbo& Driver, LabelWriterLanguageMonitor& LanguageMonitor, cups_page_header2_t& PageHeader)
{
  DriverInitializerLabelWriterTwinTurbo::processPageOptions(Driver, (DummyLanguageMonitor&)LanguageMonitor, PageHeader);
}


} // namespace
