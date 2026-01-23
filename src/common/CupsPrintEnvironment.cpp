#include <stdio.h>
#include <string>
#include "CupsPrintEnvironment.h"
#include <errno.h>
#include <cups/cups.h>
#include <cassert>

namespace DymoPrinterDriver
{

CupsPrintEnvironmentForDriver::CupsPrintEnvironmentForDriver(ILanguageMonitor& language_monitor):
  prnFile(NULL), languageMonitor(language_monitor)
{
  const char* PrnDir = getenv("DYMO_PRN_DIR");
  if (PrnDir)
  {
    std::string FileName = PrnDir;
    if (getenv("PRINTER"))
      FileName += getenv("PRINTER");
    else
      FileName += "~dymo";
    FileName += ".prn";
    prnFile = fopen(FileName.c_str(), "w+b");
  }
}

CupsPrintEnvironmentForDriver::~CupsPrintEnvironmentForDriver()
{
  if (prnFile)
    fclose(prnFile);
}


bool
CupsPrintEnvironmentForDriver::writeData(const buffer_t& data_buffer)
{
  fprintf(stderr, "DEBUG: CupsPrintEnvironmentForDriver::WriteData() buffer size is %i\n", (int)data_buffer.size());

  if (data_buffer.size())
  {
    //fwrite(&data_buffer[0], 1, data_buffer.size(), stdout);
    if (write(1, &data_buffer[0], data_buffer.size()) == -1)
    {
      fprintf(stderr, "ERROR: CupsPrintEnvironmentForDriver::WriteData() write() failed, errno=%d\n", errno);
      return false;
    }

    if (prnFile)
    {
      size_t res = fwrite(&data_buffer[0], 1, data_buffer.size(), prnFile);
      fprintf(stderr, "DEBUG: CupsPrintEnvironmentForDriver::WriteData() PRN fwrite result is %i\n", (int)res);
    }

    languageMonitor.processData(data_buffer);
  }
  return true;
}

bool
CupsPrintEnvironmentForDriver::readData(buffer_t& data_buffer)
{
  // do nothing - driver is not able to read data, only LanguageMonitor is

  data_buffer.clear();
  return true;
}

IPrintEnvironment::job_status_t
CupsPrintEnvironmentForDriver::getJobStatus()
{
  return jsOK;
}

void
CupsPrintEnvironmentForDriver::setJobStatus(job_status_t job_status)
{
}


///////////////////////////////////////////////////////////////////////
// CupsPrintEnvironmentForLanguageMonitor
///////////////////////////////////////////////////////////////////////

CupsPrintEnvironmentForLanguageMonitor::CupsPrintEnvironmentForLanguageMonitor()
{
}

CupsPrintEnvironmentForLanguageMonitor::~CupsPrintEnvironmentForLanguageMonitor()
{
}


bool
CupsPrintEnvironmentForLanguageMonitor::writeData(const buffer_t& data_buffer)
{
  fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLanguageMonitor::WriteData() buffer size is %i\n", (int)data_buffer.size());
  if (data_buffer.size())
  {
    //fwrite(&data_buffer[0], 1, data_buffer.size(), stdout);
    //fflush(stdout);
    if (write(1, &data_buffer[0], data_buffer.size()) == -1)
    {
      fprintf(stderr, "ERROR: CupsPrintEnvironmentForLanguageMonitor::WriteData() write() failed, errno=%d\n", errno);
      return false;
    }
  }
  return true;
}

bool
CupsPrintEnvironmentForLanguageMonitor::readData(buffer_t& data_buffer)
{
  //TODO: add the implementation here
  // note that CUPS 1.1 does not support reading data from the printer
  // only CUPS 1.2 supports
  // there should be API to read the 'back-channel' safely
  // also the data is avalable using read file with fd == 3

  data_buffer.clear();

  byte buf[16];
  ssize_t bytesRead = cupsBackChannelRead((char*)buf, sizeof(buf), 2.5);
  if (bytesRead == -1)
  {
    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLanguageMonitor::ReadData() unable to read data, errno=%d\n", errno);
    return false;
  }
  else if (bytesRead == 0)
  {
    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLanguageMonitor::ReadData() no data\n");
    return true; // No data is not an error
  }
  else
  {
    //data_buffer.push_back(buf[bytesRead - 1]);
    data_buffer.insert(data_buffer.begin(), buf, buf + bytesRead);

    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLanguageMonitor::ReadData() has read %i bytes %x\n", (int)bytesRead, int(data_buffer[0]));
    return true;
  }
}

IPrintEnvironment::job_status_t
CupsPrintEnvironmentForLanguageMonitor::getJobStatus()
{
  return jobStatus;
}

void
CupsPrintEnvironmentForLanguageMonitor::setJobStatus(job_status_t job_status)
{
    jobStatus = job_status;

    switch (job_status)
    {
        case jsOK:
            fputs("STATE: none\n", stderr);
            break;
        case jsPaperOut:
            fputs("STATE: com.dymo.out-of-paper-error\n", stderr);
            break;
        case jsError:
            fputs("STATE: com.dymo.general-error\n", stderr);
            break;
        case jsPaperSizeError:
            fputs("STATE: com.dymo.paper-size-error\n", stderr);
            break;
        case jsPaperSizeUndefinedError:
            fputs("STATE: com.dymo.paper-size-undefine-error\n", stderr);
            break;
        case jsHeadOverheat:
            fputs("STATE: com.dymo.head-overheat-error\n", stderr);
            break;
        case jsSlotStatusError:
            fputs("STATE: com.dymo.slot-status-error\n", stderr);
            break;
        case jsBusy:
            fputs("STATE: com.dymo.busy-error\n", stderr);
            break;
        default:
            assert(0);
    }

}

} // namespace
