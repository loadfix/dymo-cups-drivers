#ifndef CUPS_FILTER_H
#define CUPS_FILTER_H

#include <cups/cups.h>
#include <cups/raster.h>
#include <cups/ppd.h>
#include <memory>
#include <string>
#include "CupsPrintEnvironment.h"
#include "ErrorDiffusionHalftoning.h"
#include "NonLinearLaplacianHalftoning.h"
#include "CupsUtils.h"

namespace DymoPrinterDriver
{

// Generic Cups filter
// Driver - Driver class
// DriverInitializer - setup driver properties on different job steps
template<class Driver, class DriverInitializer, class LanguageMonitor>
class CupsFilter
{
public:
  CupsFilter();

  int run(int argc, char* argv[]);

private:
  void initDocument(const char* opts);

  CupsPrintEnvironmentForLanguageMonitor          printEnvironmentForLanguageMonitor;
  LanguageMonitor                     languageMonitor;
  CupsPrintEnvironmentForDriver      printEnvironmentForDriver;
  Driver                              driver;

  std::string                         halftoningMethod;
};


template <class Driver, class DriverInitializer, class LanguageMonitor>
CupsFilter<Driver, DriverInitializer, LanguageMonitor>::CupsFilter():
  printEnvironmentForLanguageMonitor(), languageMonitor(printEnvironmentForLanguageMonitor),
  printEnvironmentForDriver(languageMonitor), driver(printEnvironmentForDriver),
  halftoningMethod()
{
}

template <class Driver, class DriverInitializer, class LanguageMonitor>
int
CupsFilter<Driver, DriverInitializer, LanguageMonitor>::run(int argc, char* argv[])
{
  setbuf(stderr, NULL);

  if ((argc < 6) || (argc > 7))
  {
    fputs("ERROR: using: <filter> job-id user title copies options [file]\n", stderr);
    return (1);
  }

  int fd = 0;
  if (argc == 7)
  {
    if ((fd = open(argv[6], O_RDONLY)) == -1)
    {
      perror("ERROR: Unable to open raster file - ");
      sleep(1);
      return (1);
    }
  }
  else
    fd = 0;

  cups_raster_t* RasterData = cupsRasterOpen(fd, CUPS_RASTER_READ);
  if (!RasterData)
  {
    perror("ERROR: Unable to open raster file - ");
    return 1;
  }

  initDocument(argv[5]);

  languageMonitor.startDoc();
  driver.startDoc();

  int Page = 0;
  cups_page_header2_t PageHeader;
  while (cupsRasterReadHeader2(RasterData, &PageHeader))
  {
    ++Page;

    fprintf(stderr, "PAGE: %d 1\n", Page);
    fprintf(stderr, "DEBUG: ----- PageHeader.cupsBytesPerLine = %d\n", PageHeader.cupsBytesPerLine);
    fprintf(stderr, "DEBUG: ----- PageHeader.cupsBitsPerColor = %d\n", PageHeader.cupsBitsPerColor);
    fprintf(stderr, "DEBUG: ----- PageHeader.cupsBitsPerPixel = %d\n", PageHeader.cupsBitsPerPixel);
    fprintf(stderr, "DEBUG: ----- PageHeader.cupsColorOrder = %d\n", PageHeader.cupsColorOrder);
    fprintf(stderr, "DEBUG: ----- PageHeader.cupsHeight = %d\n", PageHeader.cupsHeight);

    buffer_t Buffer;
    Buffer.resize(PageHeader.cupsBytesPerLine, 0);

    DriverInitializer::processPageOptions(driver, languageMonitor, PageHeader);
    languageMonitor.startPage();

    if(printEnvironmentForLanguageMonitor.getJobStatus() != IPrintEnvironment::jsOK)
        break;

    driver.startPage();

    buffer_t InputLine;
    buffer_t OutputLine;
    HalftoneFilter::image_buffer_t InputImage;

    bool UseCustomHalftoning    = PageHeader.cupsBitsPerPixel > 1;
    bool IsProcessLineSupported = true;

    std::unique_ptr<HalftoneFilter> H;
    if (UseCustomHalftoning)
    {
      if (halftoningMethod == "NonLinearLaplacian")
        H.reset(new NonLinearLaplacianHalftoning(5, HalftoneFilter::itRGB, HalftoneFilter::itBW));
      else // error diffusion is default
        H.reset(new ErrorDiffusionHalftoning(HalftoneFilter::itRGB, HalftoneFilter::itBW));

      IsProcessLineSupported = H->isProcessLineSupported();
    }

    //CErrorDiffusionHalftoning H(CHalftoneFilter::itRGB, CHalftoneFilter::itBW);
    for (size_t y = 0; y < PageHeader.cupsHeight; ++y)
    {
      if ((y & 15) == 0)
        fprintf(stderr, "INFO: Printing page %d, %d%% complete...\n", Page, int(100 * y / PageHeader.cupsHeight));

      std::fill(Buffer.begin(), Buffer.end(), 0);

      size_t bytesRead = cupsRasterReadPixels(RasterData, &Buffer[0], PageHeader.cupsBytesPerLine);
      if (bytesRead != PageHeader.cupsBytesPerLine)
      {
        fprintf(stderr, "Error: cupsRasterReadPixels() failed: expected %d read, actually was %i", PageHeader.cupsBytesPerLine, (int)bytesRead);

        break;
      }

      // apply halftoning
      if (UseCustomHalftoning)
      {
        if (IsProcessLineSupported)
        {
          H->processLine(Buffer, OutputLine);
          driver.processRasterLine(OutputLine);
        }
        else
          InputImage.push_back(Buffer); // cache for later processing
      }
      else
        driver.processRasterLine(Buffer); // process line as-is, because it is already B&W

    } // all lines

    // process cached image by custom halftoning if needed
    if (UseCustomHalftoning && !IsProcessLineSupported)
    {
      HalftoneFilter::image_buffer_t OutputImage;
      H->processImage(InputImage, OutputImage);
      for (size_t i = 0; i < OutputImage.size(); ++i)
        driver.processRasterLine(OutputImage[i]);

    }

    driver.endPage();
    languageMonitor.endPage();
  }

  driver.endDoc();
  languageMonitor.endDoc();

  cupsRasterClose(RasterData);
  if (fd != 0)
    close(fd);

  if (Page == 0)
    fputs("ERROR: No pages found!\n", stderr);
  else
    fputs("INFO: Ready to print.\n", stderr);

  //fputs("DEBUG: DYMO filter hack: sending ESC A at the end\n", stderr);
  //char buf[2] = {0x1b, 'A'};
  //write(1, buf, 2);
  //close(1);

  return (Page == 0);
}

template<class Driver, class DriverInitializer, class LanguageMonitor>
void
CupsFilter<Driver, DriverInitializer, LanguageMonitor>::initDocument(const char* opts)
{
  fprintf(stderr, "DEBUG: -----------------------------options are: %s\n", opts);

  ppd_file_t* ppd = ppdOpenFile(getenv("PPD"));
  if (!ppd)
  {
    perror("WARNING: Unable to open ppd file, use default settings - ");
    return;
  }

  cups_option_t*  Options     = NULL;
  int             OptionCount = cupsParseOptions(opts, 0, &Options);

  ppdMarkDefaults(ppd);

  // mark general options as ppd options
  for (int i = 0; i < OptionCount; ++i)
  {
    if (Options[i].value)
      ppdMarkOption(ppd, Options[i].name, Options[i].value);
  }

  // do CUPS specific
  cupsMarkOptions(ppd, OptionCount, Options);

  DriverInitializer::processPPDOptions(driver, languageMonitor, ppd);

  // extract halftoning method used
  ppd_choice_t* choice = CupsUtils::findMarkedChoice(ppd, "DymoHalftoning");
  if (choice)
    halftoningMethod = choice->choice;

  cupsFreeOptions(OptionCount, Options);
  ppdClose(ppd);
}

}; // namespace

#endif // CUPS_FILTER_H
