#ifndef CUPS_PRINT_ENVIRONMENT_H
#define CUPS_PRINT_ENVIRONMENT_H

#include <cups/backend.h>
#include <cups/sidechannel.h>
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

// this is environment for a driver
// this env will be forward output to LanguageMonitor also
class CupsPrintEnvironmentForDriver: public IPrintEnvironment
{
public:
  CupsPrintEnvironmentForDriver(ILanguageMonitor& language_monitor);
  virtual ~CupsPrintEnvironmentForDriver();
  virtual bool writeData(const buffer_t& data_buffer);
  virtual bool readData(buffer_t& data_buffer);
  virtual job_status_t getJobStatus();
  virtual void setJobStatus(job_status_t job_status);

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
  virtual bool writeData(const buffer_t& data_buffer);
  virtual bool readData(buffer_t& data_buffer);
  virtual job_status_t getJobStatus();
  virtual void setJobStatus(job_status_t job_status);

private:
  FILE* PRNFile_;
  IPrintEnvironment::job_status_t JobStatus_;
};

};

#endif // CUPS_PRINT_ENVIRONMENT_H
