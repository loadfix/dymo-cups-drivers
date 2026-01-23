#include "MOCK_PrintEnvironment.h"

void
MockPrintEnvironment::WriteData(const buffer_t& Data)
{
  Data_.insert(Data_.end(), Data.begin(), Data.end());
}

void
MockPrintEnvironment::ReadData(buffer_t& Data)
{
  Data.clear();
}

IPrintEnvironment::job_status_t
MockPrintEnvironment::GetJobStatus()
{
  return jsOK;
}

void
MockPrintEnvironment::SetJobStatus(job_status_t JobStatus)
{
}

const buffer_t&
MockPrintEnvironment::GetData()
{
  return Data_;
}

void
MockPrintEnvironment::ClearData()
{
  Data_.clear();
}
