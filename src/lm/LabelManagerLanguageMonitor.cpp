// -*- C++ -*-
// $Id$

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


#include "LabelManagerLanguageMonitor.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace DymoPrinterDriver
{
    // Initialise every non-trivial POD member. TapeWidth_ and DeviceName_
    // were previously left uninitialised by the ctor; TapeWidth_ is an
    // enum (scalar) POD, reading its indeterminate value before the
    // first SetTapeWidth() call is undefined behaviour per [basic.indet].
    //
    // The sequencing matters: StartPage() on the first page calls
    // CheckStatus(), which calls CheckTapeSize(), which reads
    // TapeWidth_. But SetTapeWidth() is only invoked from
    // CDriverInitializerLabelManagerWithLM::ProcessPageOptions, which
    // CCupsFilter::Run calls AFTER StartPage. So on page 1 the monitor
    // reads TapeWidth_ before any write — pure UB without the init
    // below.
    //
    // Defaulting to tw12mm matches the most common LabelManager tape
    // cartridge and gives deterministic behaviour. DeviceName_ is a
    // std::string, which default-initialises to empty — safe either
    // way, but explicit-initialising for symmetry.
    CLabelManagerLanguageMonitor::CLabelManagerLanguageMonitor(IPrintEnvironment& Environment, bool UseSleep, size_t ReadStatusTimeout):
    Environment_(Environment),
    IsFirstPage_(true),
    PageData_(),
    DeviceName_(),
    TapeWidth_(CLabelManagerDriver::tw12mm),
    UseSleep_(UseSleep),
    LastReadStatusResult_(true),
    ReadStatusTimeout_(ReadStatusTimeout)
    {
    }

    CLabelManagerLanguageMonitor::~CLabelManagerLanguageMonitor()
    {
    }

    void
    CLabelManagerLanguageMonitor::StartDoc()
    {
        IsFirstPage_ = true;
    }

    void
    CLabelManagerLanguageMonitor::EndDoc()
    {
        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::EndDoc()\n");
    }

    void
    CLabelManagerLanguageMonitor::StartPage()
    {
        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::StartPage()\n");
        if (IsFirstPage_)
        {
            CheckStatus();
        }

        IsFirstPage_ = false;
    }

    void
    CLabelManagerLanguageMonitor::EndPage()
    {
        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::EndPage()\n");
    }

    bool
    CLabelManagerLanguageMonitor::IsLocal()
    {
        // Mirror of CLabelWriterLanguageMonitor::IsLocal() — null-check
        // the DEVICE_URI env var before handing it to strncmp. If the
        // filter is invoked outside a normal CUPS job (e.g. manual
        // invocation for debugging, or a systemd unit that doesn't
        // export DEVICE_URI), getenv returns NULL. Passing NULL to
        // strncmp is undefined behaviour per C17 §7.24.4.4/3 — on
        // glibc it SIGSEGVs on the first dereference, crashing the
        // filter on the first page's CheckStatus. Upstream DYMO got
        // this right on the LW side but forgot the guard here.
        const char* uri = getenv("DEVICE_URI");
        if (uri == NULL)
            return true;  // conservative: assume local

        return (strncmp(uri, "usb://", 6) == 0);
    }

    void
    CLabelManagerLanguageMonitor::CheckStatus()
    {
        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus()\n");

        // restore good status of the job

        SetJobStatus(CASSETTE_PRESENT_BIT);

        if(!IsLocal())
            return;

        while (true)
        {
            fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus() 1\n");

            buffer_t Status;
            time_t   BeginTime   = time(NULL);
            bool     StatusOK    = ReadStatus(Status);

            // Request status while good or bad condition or timeout.
            //
            // Upstream wrote the inner loop condition as
            //   `!StatusOK || (Status[0] & BUSY_BIT)`
            // with no check that Status.size() >= 1 when StatusOK is
            // false. ReadStatus() guarantees Status is non-empty only
            // on return-true; on return-false Status may be empty,
            // and Status[0] on an empty std::vector is undefined
            // behaviour. This mirror of Fix 2 (LW side):
            //
            //   * Adds a `Status.size() > 0` guard before Status[0].
            //   * Breaks out of the inner loop on a read-failure so
            //     we don't spin when the back-channel is dead.
            int i = 0;
            while (StatusOK
                   && Status.size() > 0
                   && (Status[0] & BUSY_BIT)
                   && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
            {
                fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus() 2 %i\n", i);

                StatusOK = ReadStatus(Status);

                i++;
            }

            if (difftime(time(NULL), BeginTime) >= ReadStatusTimeout_)
            {
                SetJobStatus(BUSY_BIT);

                fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus() timeout\n");

                break;
            }

            // Bail out of the outer loop on read-failure or an empty
            // status buffer — there's nothing useful we can interpret.
            // Without this, a completely unreadable back-channel would
            // fall through and dereference Status[0] below, then re-
            // enter the outer loop on ReprintLabel failure. Matches the
            // defensive treatment added in Fix 2 on the LW side.
            if (!StatusOK || Status.size() == 0)
            {
                fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus() read-failure-break\n");
                break;
            }

            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT && !CheckTapeSize(Status))
                Status[0] = INCORRECT_SIZE_BIT;

            if ((Status[0] == INCORRECT_SIZE_BIT) ||
                (Status[0] & GENERAL_ERROR_BIT) ||
                (Status[0] & HEAD_OVERHEAT_BIT) ||
                (Status[0] & SLOT_STATUS_BIT) ||
                ((Status[0] & CASSETTE_PRESENT_BIT) == 0))
            {
                SetJobStatus(Status[0]);
            }
            else
            {
                // restore good status of the job

                SetJobStatus(CASSETTE_PRESENT_BIT);

                break;
            }
        }

        // clear stored label data
        PageData_.clear();

        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::CheckStatus() return\n");
    }

    bool
    CLabelManagerLanguageMonitor::ReadStatus(buffer_t& Status)
    {
        time_t t = time(NULL);
        fprintf(stderr, "DEBUG: CLabelManagerLanguageMonitor::ReadStatus() %s\n", ctime(&t));

        bool Result = false;
        Status.clear();

        buffer_t RequestStatusCommand = CLabelManagerDriver::GetRequestStatusCommand();
        Environment_.WriteData(RequestStatusCommand);

        Environment_.ReadData(Status);

        if (Status.size() > 0)
        {
            Result = true;
        }

        fprintf(stderr, "DEBUG: ReadStatus() returned %i %i\n", (int)Status.size(), (int)Result);
        return Result;
    }

    void
    CLabelManagerLanguageMonitor::SetJobStatus(byte Status)
    {
        IPrintEnvironment::job_status_t JobStatus = IPrintEnvironment::jsOK;

        if (Status == INCORRECT_SIZE_BIT)
            JobStatus = IPrintEnvironment::jsPaperSizeError;
        else if (Status & GENERAL_ERROR_BIT)
            JobStatus = IPrintEnvironment::jsError;
        else if (Status & HEAD_OVERHEAT_BIT)
            JobStatus = IPrintEnvironment::jsHeadOverheat;
        else if (Status & SLOT_STATUS_BIT)
            JobStatus = IPrintEnvironment::jsSlotStatusError;
        else if (Status & BUSY_BIT)
            JobStatus = IPrintEnvironment::jsBusy;
        else if ((Status & CASSETTE_PRESENT_BIT) == 0)
            JobStatus = IPrintEnvironment::jsPaperSizeUndefinedError;

        Environment_.SetJobStatus(JobStatus);
    }

    bool
    CLabelManagerLanguageMonitor::CheckTapeSize(buffer_t Status)
    {
        fprintf(stderr, "DEBUG: CheckTapeSize() device %s tape %d\n", DeviceName_.c_str(), TapeWidth_);

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter DUO Tape 128") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter 450 DUO Tape") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 400") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PC II") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PC"))
        {
            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter DUO Tape"))
        {
            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if((((Status[0] & CASSETTE_SIZE_BITS) == 0x00) && TapeWidth_ == CLabelManagerDriver::tw6mm) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x01) && (TapeWidth_ == CLabelManagerDriver::tw9mm || TapeWidth_ == CLabelManagerDriver::tw12mm)) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x02) && TapeWidth_ == CLabelManagerDriver::tw19mm) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x03) && TapeWidth_ == CLabelManagerDriver::tw24mm))
                    return true;
                else
                    return false;
            }
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 450"))
        {
            // The LabelMANAGER 450 carries its tape-width code in the
            // SECOND status byte. ReadStatus() returns a buffer_t of
            // whatever size cupsBackChannelRead delivered — on older
            // LM 450 firmware, or on a short read, that can be one
            // byte. std::vector::operator[] is unchecked; Status[1] on
            // a size-1 vector is undefined behaviour and has been seen
            // to segfault.
            if (Status.size() < 2)
                return false;  // treat as "size doesn't match" — safe default

            if(((Status[1] & 0xFF) == 0x00) ||
               (((Status[1] & 0xFF) == 0x01) && TapeWidth_ == CLabelManagerDriver::tw6mm) ||
               (((Status[1] & 0xFF) == 0x02) && TapeWidth_ == CLabelManagerDriver::tw9mm) ||
               (((Status[1] & 0xFF) == 0x03) && TapeWidth_ == CLabelManagerDriver::tw12mm) ||
               (((Status[1] & 0xFF) == 0x04) && TapeWidth_ == CLabelManagerDriver::tw19mm) ||
               (((Status[1] & 0xFF) == 0x05) && TapeWidth_ == CLabelManagerDriver::tw24mm))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelPOINT 350"))
        {
            if((((Status[0] & CASSETTE_SIZE_BITS) == 0x01) && TapeWidth_ == CLabelManagerDriver::tw6mm) ||
               (((Status[0] & CASSETTE_SIZE_BITS) == 0x02) && (TapeWidth_ == CLabelManagerDriver::tw19mm || TapeWidth_ == CLabelManagerDriver::tw24mm)) ||
               (((Status[0] & CASSETTE_SIZE_BITS) == 0x03) && (TapeWidth_ == CLabelManagerDriver::tw9mm || TapeWidth_ == CLabelManagerDriver::tw12mm)))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PnP") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 420P") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelManager 500TS"))
        {
            // Same Status[1] short-read hazard as the LabelMANAGER 450
            // branch above — these models also encode tape-width in the
            // second status byte, and cupsBackChannelRead can return a
            // single byte on short reads or older firmware. Guard here
            // too so Status[1] is never dereferenced on an empty/1-byte
            // vector (vector::operator[] is unchecked; that would be UB).
            if (Status.size() < 2)
                return false;

            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if(((Status[1] & 0xFF) == 0x00) ||
                   (((Status[1] & 0xFF) == 0x01) && TapeWidth_ == CLabelManagerDriver::tw6mm) ||
                   (((Status[1] & 0xFF) == 0x02) && TapeWidth_ == CLabelManagerDriver::tw9mm) ||
                   (((Status[1] & 0xFF) == 0x03) && TapeWidth_ == CLabelManagerDriver::tw12mm) ||
                   (((Status[1] & 0xFF) == 0x04) && TapeWidth_ == CLabelManagerDriver::tw19mm) ||
                   (((Status[1] & 0xFF) == 0x05) && TapeWidth_ == CLabelManagerDriver::tw24mm))
                    return true;
                else
                    return false;
            }
            else
                return false;
        }

        return true;
    }

    void
    CLabelManagerLanguageMonitor::ProcessData(const buffer_t& Data)
    {
        PageData_.insert(PageData_.end(), Data.begin(), Data.end());
    }

    void
    CLabelManagerLanguageMonitor::SetDeviceName(const std::string& Value)
    {
        DeviceName_ = Value;
    }

    void
    CLabelManagerLanguageMonitor::SetTapeWidth(CLabelManagerDriver::tape_width_t Value)
    {
        TapeWidth_ = Value;
    }
}; // namespace


/*
 * End of "$Id$".
 */



