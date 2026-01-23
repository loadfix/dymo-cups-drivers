#include "MOCK_LWLMPrintEnvironment.h"
#include "../LabelWriterDriver.h"
#include <assert.h>

static const byte TOF_BIT           = 0x02;
static const byte ROLL_CHANGED_BIT  = 0x08;
static const byte PAPER_OUT_BIT     = 0x20;
static const byte PAPER_FEED_BIT    = 0x40;
static const byte ERROR_BIT         = 0x80;


MockLWLMPrintEnvironment::MockLWLMPrintEnvironment(): Mode_()
{
  Mode_.push_back(mtTOF);
}

void
MockLWLMPrintEnvironment::WriteData(const buffer_t& Data)
{
  //    printf("MockLWLMPrintEnvironment::WriteData()");

  // don't store request status requests
  if (Data != CLabelWriterDriver::GetRequestStatusCommand())
    MockPrintEnvironment::WriteData(Data);
}

void
MockLWLMPrintEnvironment::ReadData(buffer_t& Data)
{
  //printf("MockLWLMPrintEnvironment::ReadData()");

  Data.clear();

  mode_t Mode = mtTOF;

  if (Mode_.size() > 1)
  {
    Mode = Mode_.back();
    Mode_.pop_back();
  }
  else
    Mode = Mode_.back();

  switch (Mode)
  {
    case mtTOF:
      Data.push_back(TOF_BIT);
      break;
    case mtPaperOut:
      Data.push_back(ERROR_BIT | PAPER_OUT_BIT);
      break;
    case mtRollChanged:
      Data.push_back(TOF_BIT | ROLL_CHANGED_BIT);
      break;
    case mtNotTOF:
      Data.push_back(0);
      break;
    case mtFailed:
      break;
    default:
      assert(0);
  }
}

IPrintEnvironment::job_status_t
MockLWLMPrintEnvironment::GetJobStatus()
{
  return jsOK;
}


void
MockLWLMPrintEnvironment::SetJobStatus(job_status_t JobStatus)
{
  switch (JobStatus)
  {
    case jsOK:
      //fprintf(stderr, "INFO: continue printing.\n");
      break;
    case jsPaperOut:
      //fprintf(stderr, "INFO: paper out.\n");
      break;
    case jsError:
      //fprintf(stderr, "INFO: printing error.\n");
      break;
    default:
      assert(0);
  }
}


void
MockLWLMPrintEnvironment::SetMode(mode_t Mode)
{
  Mode_.clear();
  Mode_.push_back(Mode);
}

void
MockLWLMPrintEnvironment::PushMode(mode_t Mode, size_t Count)
{
  for (size_t i = 0; i < Count; ++i)
    Mode_.push_back(Mode);
}
