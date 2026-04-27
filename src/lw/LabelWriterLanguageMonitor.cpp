// -*- C++ -*-
// $Id: LabelWriterLanguageMonitor.cpp 15965 2011-09-02 14:48:46Z pineichen $

// DYMO LabelWriter Drivers
// Copyright (C) 2008 Sanford L.P.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


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
CLabelWriterLanguageMonitor::CLabelWriterLanguageMonitor(IPrintEnvironment& Environment, bool UseSleep, size_t ReadStatusTimeout):
  Environment_(Environment), PaperType_(CLabelWriterDriver::ptRegular), Roll_(CLabelWriterDriverTwinTurbo::rtAuto), RollUsed_(false), IsFirstPage_(true), PageData_(), UseSleep_(UseSleep), LastStatus_(0), LastReadStatusResult_(true), ReadStatusTimeout_(ReadStatusTimeout)
{
}

CLabelWriterLanguageMonitor::~CLabelWriterLanguageMonitor()
{
}
    
void 
CLabelWriterLanguageMonitor::StartDoc()
{
  IsFirstPage_ = true;
  ResetPrinter();

  if (RollUsed_)
    SynchronizeRoll();
}

void 
CLabelWriterLanguageMonitor::EndDoc()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::EndDoc()\n");
  //CheckStatusAndReprint();
}

void 
CLabelWriterLanguageMonitor::StartPage()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::StartPage()\n");
  if (IsFirstPage_)
  {
    CheckStatusAndReprint();
  }
    
  IsFirstPage_ = false;
}

void 
CLabelWriterLanguageMonitor::EndPage()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::EndPage()\n");
  CheckStatusAndReprint();
}

bool
CLabelWriterLanguageMonitor::IsLocal()
{
  bool bIsLocal = true;

  char* uri = getenv("DEVICE_URI");

  if(uri != NULL)  
    bIsLocal = (strncmp(uri, "usb://", 6) == 0);

  return bIsLocal;
}

void
CLabelWriterLanguageMonitor::SynchronizeRoll()
{
  buffer_t buf = CLabelWriterDriverTwinTurbo::GetRollSelectCommand(Roll_);
  Environment_.WriteData(buf);
}

void
CLabelWriterLanguageMonitor::ResetPrinter()
{
  buffer_t buf = CLabelWriterDriver::GetResetCommand();
  Environment_.WriteData(buf);
}


void
CLabelWriterLanguageMonitor::CheckStatusAndReprint()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint()\n");

  // restore good status of the job
  SetJobStatus(TOF_BIT);

  if(!IsLocal())
      return;

  while (true) // reprint also can fail, so don't forget to recheck status after reprint
  {
    fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint() 1\n");

    byte    Status      = 0;
    time_t  BeginTime   = time(NULL);
    bool    StatusOK    = ReadStatus(Status);

    // Request status while neither a good condition (TOF), a bad condition
    // (ERROR / ROLL_CHANGED), a read failure, nor the wall-clock timeout has
    // been reached.
    //
    // Upstream DYMO commented out the `StatusOK` read-failure guard (see git
    // history at aac3cb6 / a062d61 of src/lw/LabelWriterLanguageMonitor.cpp).
    // With Status initialised to 0 and `StatusOK` ignored, the inner-loop
    // condition `!((Status & TOF_BIT) || ...)` is always true until either
    // the printer finally ACKs or the ReadStatusTimeout_ (default 10s)
    // wall-clock elapses. On a healthy USB link the ACK arrives within a few
    // ms, but a silent back-channel causes a full 10s busy-spin on every
    // single page — and every iteration writes an ESC-A request byte into
    // the print stdout stream, polluting the USB pipeline.
    //
    // Reinstating the `StatusOK` guard makes the loop exit immediately on
    // an unreadable back-channel (cupsBackChannelRead returning -1 or 0
    // bytes), converting a 10-second hang into an O(100ms) noop. Correct
    // status bits still win the race when the printer responds promptly.
    int i = 0;
    while (
      StatusOK
      && !((Status & TOF_BIT) || (Status & ERROR_BIT) || (Status & ROLL_CHANGED_BIT))
      && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
    {
      fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint() 2 %i\n", i);
      StatusOK = ReadStatus(Status);
      //usleep(100000);
      i++;
    }

    if (difftime(time(NULL), BeginTime) >= ReadStatusTimeout_)
    {
      fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint() timeout\n");
      break;
    }

    if (!StatusOK)
    {
      // Read failure — back-channel is gone or the printer isn't answering.
      // Nothing useful we can do; fall out so EndPage can return.
      fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint() read-failure-break\n");
      break;
    }
    
    // error - needs reprint
    if ((Status & ERROR_BIT) || (Status & ROLL_CHANGED_BIT) || !(Status & TOF_BIT))
    {
      // force error bit in case of timeout
      if (!(Status & TOF_BIT) && !(Status & ROLL_CHANGED_BIT))
        Status |= ERROR_BIT;

      SetJobStatus(Status);
      if (PollUntilPaperIn())
      {   
        // restore good status of the job
        SetJobStatus(TOF_BIT);

        ReprintLabel();
      }
    }
    else
      break;
  }

  // clear stored label data
  PageData_.clear();

  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::CheckStatusAndReprint() return\n");
}

bool
CLabelWriterLanguageMonitor::ReadStatus(byte& Status)
{
  time_t t = time(NULL);
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::ReadStatus() %s\n", ctime(&t));

  bool Result = false;
  Status = 0; // default
  LastStatus_ = 0;

  //if (LastReadStatusResult_)
  {
    buffer_t RequestStatusCommand = CLabelWriterDriver::GetRequestStatusCommand();
    Environment_.WriteData(RequestStatusCommand);
  }
  //Environment_.WriteData(buffer_t(128, 0));
  //Environment_.WriteData(RequestStatusCommand);
    
  //byte b[] = {
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A',
  //    0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A', 0x1b, 'A'};
  //byte b[] = { 0x1b, 'A', 0x1b, 'A'};
  //Environment_.WriteData(buffer_t(b, b + sizeof(b))); 
    
  buffer_t buf;
  Environment_.ReadData(buf);

  if (buf.size() > 0)
  {
    // cupsBackChannelRead may return up to 16 bytes in a single call — one
    // byte per ESC-A request we have previously issued. DYMO's original
    // code kept only the *first* byte of the buffer, leaving any late-
    // arriving earlier replies to pollute the next ReadData() call. Use
    // the *last* byte instead: it corresponds to the most recent ESC-A we
    // sent in this iteration, which is the status we actually want to
    // reason about. All earlier bytes are stale observations of the
    // printer state before we issued the current request.
    Status = buf[buf.size() - 1];

    if (PaperType_ == CLabelWriterDriver::ptContinuous)
      Status |= TOF_BIT;

    Result = true;
  }

  //if (!LastReadStatusResult_ && Result)
  //{
  //  LastReadStatusResult_ = true;
  //  Result = false;
  // }
  //else
  //  LastReadStatusResult_ = Result;
  
  LastStatus_ = Status;

  fprintf(stderr, "DEBUG: ReadStatus() returned %x %i\n", Status, (int)Result);
  return Result;
}

bool
CLabelWriterLanguageMonitor::PollUntilPaperIn()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::PollUntilPaperIn()\n");

  byte Status = 0;
  // The original DYMO source used `for(;;)` with the only escape clauses
  // being `GetJobStatus() == jsDeleted` and `ReadStatus() == false`.
  // The jsDeleted escape is inert on this platform: the only place that
  // status is ever written is inside CupsFilter.h when the scheduler
  // cancels a job, but on modern CUPS the cancel arrives as SIGTERM on
  // the filter process rather than as a state write that this loop can
  // observe. So in practice only ReadStatus=false exits the loop, and
  // when the back-channel ACKs with a "still waiting for paper" byte we
  // can spin for an unbounded time.
  //
  // Cap total wait at ReadStatusTimeout_ seconds (already the configured
  // wall-clock budget for CheckStatusAndReprint) so a stuck printer can't
  // hold the filter open forever.
  const time_t pollDeadline = time(NULL) + (time_t)ReadStatusTimeout_;

  for(;;)
  {
    // TODO: use platform-independent call
    if (UseSleep_)
    {
      //sleep(2);
      timespec interval;
      interval.tv_sec = 0;
      interval.tv_nsec = 200000000; // 0.2 second
      nanosleep(&interval, NULL);
    }

    if (time(NULL) >= pollDeadline)
    {
      fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::PollUntilPaperIn() deadline-exceeded\n");
      return false;
    }

    if (Environment_.GetJobStatus() == IPrintEnvironment::jsDeleted)
      return false;

    if (!ReadStatus(Status))
      return false;

    SetJobStatus(Status); // update status

    if ((Status & TOF_BIT) && !(Status & ERROR_BIT))
      return true;
  }
}

void
CLabelWriterLanguageMonitor::SetJobStatus(byte Status)
{
  // Translate the printer's ESC-A status byte into the CUPS-facing
  // job_status_t enum. Precedence matters: the spec (p.10) notes that
  // the Error bit (0x80) is set whenever bit 5 or bit 6 is set, so
  // we must test for paper-out and paper-jam specifically before
  // falling back to the generic jsError.
  //
  // Paper Jam (bit 6 / 0x40) surfaces through CUPS as
  // com.dymo.slot-status-error, which the LW 450-family PPDs already
  // declare (e.g. ppd/lw450.ppd line ~51). Upstream DYMO named this
  // bit "PAPER_FEED_BIT" and folded it into jsPaperOut, causing users
  // to see a physical jam reported as "out of paper" and try to fix
  // it by loading more labels. Distinguishing the two lets CUPS show
  // the correct reason and lets the user take the right action.
  IPrintEnvironment::job_status_t JobStatus = IPrintEnvironment::jsOK;

  if (Status & PAPER_OUT_BIT)
    JobStatus = IPrintEnvironment::jsPaperOut;
  else if (Status & PAPER_JAM_BIT)
    JobStatus = IPrintEnvironment::jsSlotStatusError;
  else if (Status & ERROR_BIT)
    JobStatus = IPrintEnvironment::jsError;

  Environment_.SetJobStatus(JobStatus);
}

void
CLabelWriterLanguageMonitor::ReprintLabel()
{
  fprintf(stderr, "DEBUG: CLabelWriterLanguageMonitor::ReprintLabel()\n");

  // send form feed first
  if (!(LastStatus_ & ROLL_CHANGED_BIT))
  {
    buffer_t ShortFormFeedCommand = CLabelWriterDriver400::GetShortFormFeedCommand();
    Environment_.WriteData(ShortFormFeedCommand);
  }
    
  Environment_.WriteData(PageData_);
}
    
void 
CLabelWriterLanguageMonitor::ProcessData(const buffer_t& Data)
{
  PageData_.insert(PageData_.end(), Data.begin(), Data.end());
}

void        
CLabelWriterLanguageMonitor::SetPaperType(CLabelWriterDriver::paper_type_t Value)
{
  PaperType_ = Value;
}

void        
CLabelWriterLanguageMonitor::SetRoll(CLabelWriterDriverTwinTurbo::roll_t Value)
{
  Roll_       = Value;
  RollUsed_   = true;
}


}; // namespace


/*
 * End of "$Id: LabelWriterLanguageMonitor.cpp 15965 2011-09-02 14:48:46Z pineichen $".
 */



