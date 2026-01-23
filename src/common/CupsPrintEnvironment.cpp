#include <stdio.h>
#include <string>
#include "CupsPrintEnvironment.h"
#include <errno.h>
#include <cups/cups.h>
#include <cassert>

namespace DymoPrinterDriver
{

CupsPrintEnvironmentForDriver::CupsPrintEnvironmentForDriver(ILanguageMonitor& LanguageMonitor):
  PRNFile_(NULL), LanguageMonitor_(LanguageMonitor)
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
    PRNFile_ = fopen(FileName.c_str(), "w+b");
  }
}

CupsPrintEnvironmentForDriver::~CupsPrintEnvironmentForDriver()
{
  if (PRNFile_)
    fclose(PRNFile_);
}


bool
CupsPrintEnvironmentForDriver::WriteData(const buffer_t& DataBuffer)
{
  fprintf(stderr, "DEBUG: CupsPrintEnvironmentForDriver::WriteData() buffer size is %i\n", (int)DataBuffer.size());

  if (DataBuffer.size())
  {
    //fwrite(&DataBuffer[0], 1, DataBuffer.size(), stdout);
    if (write(1, &DataBuffer[0], DataBuffer.size()) == -1)
    {
      fprintf(stderr, "ERROR: CupsPrintEnvironmentForDriver::WriteData() write() failed, errno=%d\n", errno);
      return false;
    }

    if (PRNFile_)
    {
      size_t res = fwrite(&DataBuffer[0], 1, DataBuffer.size(), PRNFile_);
      fprintf(stderr, "DEBUG: CupsPrintEnvironmentForDriver::WriteData() PRN fwrite result is %i\n", (int)res);
    }

    LanguageMonitor_.ProcessData(DataBuffer);
  }
  return true;
}

bool
CupsPrintEnvironmentForDriver::ReadData(buffer_t& DataBuffer)
{
  // do nothing - driver is not able to read data, only LM is

  DataBuffer.clear();
  return true;
}

IPrintEnvironment::job_status_t
CupsPrintEnvironmentForDriver::GetJobStatus()
{
  return jsOK;
}

void
CupsPrintEnvironmentForDriver::SetJobStatus(job_status_t JobStatus)
{
}


///////////////////////////////////////////////////////////////////////
// CCupsPrintEnvironmentForLM
///////////////////////////////////////////////////////////////////////

CupsPrintEnvironmentForLM::CupsPrintEnvironmentForLM()
{
}

CupsPrintEnvironmentForLM::~CupsPrintEnvironmentForLM()
{
}


bool
CupsPrintEnvironmentForLM::WriteData(const buffer_t& DataBuffer)
{
  fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLM::WriteData() buffer size is %i\n", (int)DataBuffer.size());
  if (DataBuffer.size())
  {
    //fwrite(&DataBuffer[0], 1, DataBuffer.size(), stdout);
    //fflush(stdout);
    if (write(1, &DataBuffer[0], DataBuffer.size()) == -1)
    {
      fprintf(stderr, "ERROR: CupsPrintEnvironmentForLM::WriteData() write() failed, errno=%d\n", errno);
      return false;
    }
  }
  return true;
}

bool
CupsPrintEnvironmentForLM::ReadData(buffer_t& DataBuffer)
{
  //TODO: add the implementation here
  // note that CUPS 1.1 does not support reading data from the printer
  // only CUPS 1.2 supports
  // there should be API to read the 'back-channel' safely
  // also the data is avalable using read file with fd == 3

  DataBuffer.clear();

  byte buf[16];
  ssize_t bytesRead = cupsBackChannelRead((char*)buf, sizeof(buf), 2.5);
  if (bytesRead == -1)
  {
    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLM::ReadData() unable to read data, errno=%d\n", errno);
    return false;
  }
  else if (bytesRead == 0)
  {
    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLM::ReadData() no data\n");
    return true; // No data is not an error
  }
  else
  {
    //DataBuffer.push_back(buf[bytesRead - 1]);
    DataBuffer.insert(DataBuffer.begin(), buf, buf + bytesRead);

    fprintf(stderr, "DEBUG: CupsPrintEnvironmentForLM::ReadData() has read %i bytes %x\n", (int)bytesRead, int(DataBuffer[0]));
    return true;
  }
}

IPrintEnvironment::job_status_t
CupsPrintEnvironmentForLM::GetJobStatus()
{
  return JobStatus_;
}

void
CupsPrintEnvironmentForLM::SetJobStatus(job_status_t JobStatus)
{
    JobStatus_ = JobStatus;

    switch (JobStatus)
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
