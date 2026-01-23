#ifndef MOCK_PRINT_ENVIRONMENT_H
#define MOCK_PRINT_ENVIRONMENT_H

#include "../PrinterDriver.h"

using namespace DymoPrinterDriver;

class MockPrintEnvironment: public IPrintEnvironment
{
public:
  virtual ~MockPrintEnvironment() {}
  virtual void WriteData(const buffer_t& Data);
  virtual void ReadData(buffer_t& Data);

  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

  const buffer_t& GetData();

  void ClearData();
private:
  buffer_t Data_;
};

#endif // MOCK_PRINT_ENVIRONMENT_H
