#ifndef MOCK_LWLM_PRINT_ENVIRONMENT_H
#define MOCK_LWLM_PRINT_ENVIRONMENT_H

#include <stdlib.h>
#include "../PrinterDriver.h"
#include "MOCK_PrintEnvironment.h"

using namespace DymoPrinterDriver;



class MockLWLMPrintEnvironment: public MockPrintEnvironment
{
public:
  typedef enum
  {
    mtTOF,           // return TOF
    mtPaperOut,      // return paper out
    mtNotTOF,        // not TOF but ether not ERROR
    mtFailed,        // unable to read status
    mtRollChanged

  }mode_t;

  MockLWLMPrintEnvironment();
  virtual ~MockLWLMPrintEnvironment() {}
  virtual void WriteData(const buffer_t& Data);
  virtual void ReadData(buffer_t& Data);
  virtual job_status_t GetJobStatus();
  virtual void SetJobStatus(job_status_t JobStatus);

  //////////////////////////////////////////////////////
  // functions to control what is returned by requests

  // return Mode alwayes
  void SetMode(mode_t Mode);

  // push mode that will be returned next call to ReadData()
  void PushMode(mode_t Mode, size_t Count = 1);

private:
  std::vector<mode_t> Mode_;
};

#endif // MOCK_LWLM_PRINT_ENVIRONMENT_H
