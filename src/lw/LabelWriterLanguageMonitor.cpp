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
  environment(environment), paperType(LabelWriterDriver::PAPER_TYPE_REGULAR), roll(LabelWriterDriverTwinTurbo::ROLL_AUTO), rollUsed(false), isFirstPage(true), pageData(), useSleep(use_sleep), lastStatus(0), lastReadStatusResult(true), readStatusTimeout(read_status_timeout)
{
}

LabelWriterLanguageMonitor::~LabelWriterLanguageMonitor()
{
}

void
LabelWriterLanguageMonitor::startDoc()
{
  isFirstPage = true;
  resetPrinter();

  if (rollUsed)
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
  if (isFirstPage)
  {
    checkStatusAndReprint();
  }

  isFirstPage = false;
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
  buffer_t buffer = LabelWriterDriverTwinTurbo::getRollSelectCommand(roll);
  environment.writeData(buffer);
}

void
LabelWriterLanguageMonitor::resetPrinter()
{
  buffer_t buffer = LabelWriterDriver::getResetCommand();
  environment.writeData(buffer);
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

    byte    status      = 0;
    time_t  beginTime   = time(NULL);
//    bool    statusOK    = readStatus(status);

    // request status while good or bad condition or timeout
    int i = 0;
    while (
      //statusOK
     !((status & TOF_BIT) || (status & ERROR_BIT) || (status & ROLL_CHANGED_BIT))
      && (difftime(time(NULL), beginTime) < readStatusTimeout))
    {
      fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() 2 %i\n", i);
//      statusOK =
      readStatus(status);
      //usleep(100000);
      i++;
    }

    if (difftime(time(NULL), beginTime) >= readStatusTimeout)
    {
      fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() timeout\n");
      break;
    }

    //if (!statusOK)
    //{
    //  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() 3\n");
    //  break;
    //}

    // error - needs reprint
    if ((status & ERROR_BIT) || (status & ROLL_CHANGED_BIT) || !(status & TOF_BIT))
    {
      // force error bit in case of timeout
      if (!(status & TOF_BIT) && !(status & ROLL_CHANGED_BIT))
        status |= ERROR_BIT;

      setJobStatus(status);
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
  pageData.clear();

  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::CheckStatusAndReprint() return\n");
}

bool
LabelWriterLanguageMonitor::readStatus(byte& status)
{
  time_t t = time(NULL);
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::ReadStatus() %s\n", ctime(&t));

  bool result = false;
  status = 0; // default
  lastStatus = 0;

  //if (lastReadStatusResult)
  {
    buffer_t requestStatusCommand = LabelWriterDriver::getRequestStatusCommand();
    environment.writeData(requestStatusCommand);
  }
  //environment_.writeData(buffer_t(128, 0));
  //environment.writeData(requestStatusCommand);

  //byte b[] = {
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A',
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A'};
  //byte b[] = { 0x1b, 'A', 0x1b, 'A'};
  //environment_.writeData(buffer_t(b, b + sizeof(b)));

  buffer_t buffer;
  environment.readData(buffer);

  if (buffer.size() > 0)
  {

    status = buffer[0];

    if (paperType == LabelWriterDriver::PAPER_TYPE_CONTINUOUS)
      status |= TOF_BIT;

    result = true;
  }

  //if (!lastReadStatusResult && result)
  //{
  //  lastReadStatusResult = true;
  //  result = false;
  // }
  //else
  //  lastReadStatusResult = result;

  lastStatus = status;

  fprintf(stderr, "DEBUG: readStatus() returned %x %i\n", status, (int)result);
  return result;
}

bool
LabelWriterLanguageMonitor::pollUntilPaperIn()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::pollUntilPaperIn()\n");

  byte status = 0;
  for(;;)
  {
    // TODO: use platform-undependend call
    if (useSleep)
    {
      //sleep(2);
      timespec interval;
      interval.tv_sec = 0;
      interval.tv_nsec = 200000000; // 0.2 second
      nanosleep(&interval, NULL);
    }

    if (environment.getJobStatus() == IPrintEnvironment::jsDeleted)
      return false;

    if (!readStatus(status))
      return false;

    setJobStatus(status); // update status

    if ((status & TOF_BIT) && !(status & ERROR_BIT))
      return true;
  }
}

void
LabelWriterLanguageMonitor::setJobStatus(byte status)
{
  IPrintEnvironment::job_status_t jobStatus = IPrintEnvironment::jsOK;

  if ((status & PAPER_OUT_BIT) || (status & PAPER_FEED_BIT))
    jobStatus = IPrintEnvironment::jsPaperOut;
  else if (status & ERROR_BIT)
    jobStatus = IPrintEnvironment::jsError;

  environment.setJobStatus(jobStatus);
}

void
LabelWriterLanguageMonitor::reprintLabel()
{
  fprintf(stderr, "DEBUG: LabelWriterLanguageMonitor::reprintLabel()\n");

  // send form feed first
  if (!(lastStatus & ROLL_CHANGED_BIT))
  {
    buffer_t shortFormFeedCommand = LabelWriterDriver400::getShortFormFeedCommand();
    environment.writeData(shortFormFeedCommand);
  }

  environment.writeData(pageData);
}

void
LabelWriterLanguageMonitor::processData(const buffer_t& data)
{
  pageData.insert(pageData.end(), data.begin(), data.end());
}

void
LabelWriterLanguageMonitor::setPaperType(LabelWriterDriver::paper_type_t value)
{
  paperType = value;
}

void
LabelWriterLanguageMonitor::setRoll(LabelWriterDriverTwinTurbo::roll_t value)
{
  roll       = value;
  rollUsed   = true;
}


}; // namespace
