#ifndef CUPS_PRINT_ENVIRONMENT_H
#define CUPS_PRINT_ENVIRONMENT_H

#include <cups/backend.h>
#include <cups/sidechannel.h>
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

// this is environment for a driver
// this env will be forward output to LM also
class CupsPrintEnvironmentForDriver: public IPrintEnvironment
{
public:
  CupsPrintEnvironmentForDriver(ILanguageMonitor& LanguageMonitor);
  virtual ~CupsPrintEnvironmentForDriver();
  virtual bool WriteData(const buffer_t& DataBuffer);
  virtual bool ReadData(buffer_t& DataBuffer);
  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

private:
  FILE* PRNFile_;
  ILanguageMonitor& LanguageMonitor_;
};

// this is environment for a language monitor
// it simple output it is data to CUPS file descriptor
class CupsPrintEnvironmentForLM: public IPrintEnvironment
{
public:
  CupsPrintEnvironmentForLM();
  virtual ~CupsPrintEnvironmentForLM();
  virtual bool WriteData(const buffer_t& DataBuffer);
  virtual bool ReadData(buffer_t& DataBuffer);
  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

private:
  FILE* PRNFile_;
  IPrintEnvironment::job_status_t JobStatus_;
};

};

#endif // CUPS_PRINT_ENVIRONMENT_H
