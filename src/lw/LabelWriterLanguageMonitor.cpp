#include "LabelWriterLanguageMonitor.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace DymoPrinterDriver
{

//const byte ESC = 0x1B;
LabelWriterLanguageMonitor::LabelWriterLanguageMonitor(IPrintEnvironment& environment, bool use_sleep, size_t read_status_timeout):
  Environment_(environment), PaperType_(LabelWriterDriver::ptRegular), Roll_(LabelWriterDriverTwinTurbo::rtAuto), RollUsed_(false), IsFirstPage_(true), PageData_(), UseSleep_(use_sleep), LastStatus_(0), LastReadStatusResult_(true), ReadStatusTimeout_(read_status_timeout)
{
}

LabelWriterLanguageMonitor::~LabelWriterLanguageMonitor()
{
}

void
LabelWriterLanguageMonitor::startDoc()
{
  IsFirstPage_ = true;
  resetPrinter();

  if (RollUsed_)
    synchronizeRoll();
}

void
LabelWriterLanguageMonitor::endDoc()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::endDoc()\n");
  //checkStatusAndReprint();
}

void
LabelWriterLanguageMonitor::startPage()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::startPage()\n");
  if (IsFirstPage_)
  {
    checkStatusAndReprint();
  }

  IsFirstPage_ = false;
}

void
LabelWriterLanguageMonitor::endPage()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::endPage()\n");
  checkStatusAndReprint();
}

bool
LabelWriterLanguageMonitor::isLocal()
{
  bool bIsLocal = true;

  char* uri = getenv("DEVICE_URI");

  if(uri != NULL)
    bIsLocal = (strncmp(uri, "usb://", 6) == 0);

  return bIsLocal;
}

void
LabelWriterLanguageMonitor::synchronizeRoll()
{
  buffer_t buf = LabelWriterDriverTwinTurbo::getRollSelectCommand(Roll_);
  Environment_.writeData(buf);
}

void
LabelWriterLanguageMonitor::resetPrinter()
{
  buffer_t buf = LabelWriterDriver::getResetCommand();
  Environment_.writeData(buf);
}


void
LabelWriterLanguageMonitor::checkStatusAndReprint()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint()\n");

  // restore good status of the job
  setJobStatus(TOF_BIT);

    if(!isLocal())
      return;

  while (true) // reprint also can fail, so don't forget to recheck status after reprint
  {
    fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() 1\n");

    byte    Status      = 0;
    time_t  BeginTime   = time(NULL);
//    bool    StatusOK    = ReadStatus(Status);

    // request status while good or bad condition or timeout
    int i = 0;
    while (
      //StatusOK
     !((Status & TOF_BIT) || (Status & ERROR_BIT) || (Status & ROLL_CHANGED_BIT))
      && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
    {
      fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() 2 %i\n", i);
//      StatusOK =
      readStatus(Status);
      //usleep(100000);
      i++;
    }

    if (difftime(time(NULL), BeginTime) >= ReadStatusTimeout_)
    {
      fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() timeout\n");
      break;
    }

    //if (!StatusOK)
    //{
    //  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() 3\n");
    //  break;
    //}

    // error - needs reprint
    if ((Status & ERROR_BIT) || (Status & ROLL_CHANGED_BIT) || !(Status & TOF_BIT))
    {
      // force error bit in case of timeout
      if (!(Status & TOF_BIT) && !(Status & ROLL_CHANGED_BIT))
        Status |= ERROR_BIT;

      setJobStatus(Status);
      if (pollUntilPaperIn())
      {
        // restore good status of the job
        setJobStatus(TOF_BIT);

        reprintLabel();
      }
    }
    else
      break;
  }

  // clear stored label data
  PageData_.clear();

  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() return\n");
}

bool
LabelWriterLanguageMonitor::readStatus(byte& status)
{
  time_t t = time(NULL);
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::ReadStatus() %s\n", ctime(&t));

  bool Result = false;
  status = 0; // default
  LastStatus_ = 0;

  //if (LastReadStatusResult_)
  {
    buffer_t RequestStatusCommand = LabelWriterDriver::getRequestStatusCommand();
    Environment_.writeData(RequestStatusCommand);
  }
  //Environment_.writeData(buffer_t(128, 0));
  //Environment_.writeData(RequestStatusCommand);

  //byte b[] = {
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A',
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A'};
  //byte b[] = { 0x1b, 'A', 0x1b, 'A'};
  //Environment_.writeData(buffer_t(b, b + sizeof(b)));

  buffer_t buf;
  Environment_.readData(buf);

  if (buf.size() > 0)
  {

    status = buf[0];

    if (PaperType_ == LabelWriterDriver::ptContinuous)
      status |= TOF_BIT;

    Result = true;
  }

  //if (!LastReadStatusResult_ && Result)
  //{
  //  LastReadStatusResult_ = true;
  //  Result = false;
  // }
  //else
  //  LastReadStatusResult_ = Result;

  LastStatus_ = status;

  fprintf(stderr, "DEBUG: readStatus() returned %x %i\n", status, (int)Result);
  return Result;
}

bool
LabelWriterLanguageMonitor::pollUntilPaperIn()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::pollUntilPaperIn()\n");

  byte Status = 0;
  for(;;)
  {
    // TODO: use platform-undependend call
    if (UseSleep_)
    {
      //sleep(2);
      timespec interval;
      interval.tv_sec = 0;
      interval.tv_nsec = 200000000; // 0.2 second
      nanosleep(&interval, NULL);
    }

    if (Environment_.getJobStatus() == IPrintEnvironment::jsDeleted)
      return false;

    if (!readStatus(Status))
      return false;

    setJobStatus(Status); // update status

    if ((Status & TOF_BIT) && !(Status & ERROR_BIT))
      return true;
  }
}

void
LabelWriterLanguageMonitor::setJobStatus(byte Status)
{
  IPrintEnvironment::job_status_t JobStatus = IPrintEnvironment::jsOK;

  if ((Status & PAPER_OUT_BIT) || (Status & PAPER_FEED_BIT))
    JobStatus = IPrintEnvironment::jsPaperOut;
  else if (Status & ERROR_BIT)
    JobStatus = IPrintEnvironment::jsError;

  Environment_.setJobStatus(JobStatus);
}

void
LabelWriterLanguageMonitor::reprintLabel()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::reprintLabel()\n");

  // send form feed first
  if (!(LastStatus_ & ROLL_CHANGED_BIT))
  {
    buffer_t ShortFormFeedCommand = LabelWriterDriver400::getShortFormFeedCommand();
    Environment_.writeData(ShortFormFeedCommand);
  }

  Environment_.writeData(PageData_);
}

void
LabelWriterLanguageMonitor::processData(const buffer_t& data)
{
  PageData_.insert(PageData_.end(), data.begin(), data.end());
}

void
LabelWriterLanguageMonitor::setPaperType(LabelWriterDriver::paper_type_t value)
{
  PaperType_ = value;
}

void
LabelWriterLanguageMonitor::setRoll(LabelWriterDriverTwinTurbo::roll_t value)
{
  Roll_       = value;
  RollUsed_   = true;
}


}; // namespace
