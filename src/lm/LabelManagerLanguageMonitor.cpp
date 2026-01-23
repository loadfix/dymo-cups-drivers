#include "LabelManagerLanguageMonitor.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace DymoPrinterDriver
{
    LabelManagerLanguageMonitor::LabelManagerLanguageMonitor(IPrintEnvironment& Environment, bool UseSleep, size_t ReadStatusTimeout):
    Environment_(Environment), IsFirstPage_(true), PageData_(), UseSleep_(UseSleep), LastReadStatusResult_(true), ReadStatusTimeout_(ReadStatusTimeout)
    {
    }

    LabelManagerLanguageMonitor::~LabelManagerLanguageMonitor()
    {
    }

    void
    LabelManagerLanguageMonitor::StartDoc()
    {
        IsFirstPage_ = true;
    }

    void
    LabelManagerLanguageMonitor::EndDoc()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::EndDoc()\n");
    }

    void
    LabelManagerLanguageMonitor::StartPage()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::StartPage()\n");
        if (IsFirstPage_)
        {
            CheckStatus();
        }

        IsFirstPage_ = false;
    }

    void
    LabelManagerLanguageMonitor::EndPage()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::EndPage()\n");
    }

    bool
    LabelManagerLanguageMonitor::IsLocal()
    {
        char* uri = getenv("DEVICE_URI");

        return (strncmp(uri, "usb://", 6) == 0);
    }

    void
    LabelManagerLanguageMonitor::CheckStatus()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::CheckStatus()\n");

        // restore good status of the job

        SetJobStatus(CASSETTE_PRESENT_BIT);

        if(!IsLocal())
            return;

        while (true)
        {
            fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::CheckStatus() 1\n");

            buffer_t Status;
            time_t   BeginTime   = time(NULL);
            bool     StatusOK    = ReadStatus(Status);

            // request status while good or bad condition or timeout
            int i = 0;
            while ((!StatusOK || (Status[0] & BUSY_BIT)) && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
            {
                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::CheckStatus() 2 %i\n", i);

                StatusOK = ReadStatus(Status);

                i++;
            }

            if (difftime(time(NULL), BeginTime) >= ReadStatusTimeout_)
            {
                SetJobStatus(BUSY_BIT);

                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::CheckStatus() timeout\n");

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

        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::CheckStatus() return\n");
    }

    bool
    LabelManagerLanguageMonitor::ReadStatus(buffer_t& Status)
    {
        time_t t = time(NULL);
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::ReadStatus() %s\n", ctime(&t));

        bool Result = false;
        Status.clear();

        buffer_t RequestStatusCommand = LabelManagerDriver::GetRequestStatusCommand();
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
    LabelManagerLanguageMonitor::SetJobStatus(byte Status)
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
    LabelManagerLanguageMonitor::CheckTapeSize(buffer_t Status)
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
                if((((Status[0] & CASSETTE_SIZE_BITS) == 0x00) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x01) && (TapeWidth_ == LabelManagerDriver::tw9mm || TapeWidth_ == LabelManagerDriver::tw12mm)) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x02) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
                   (((Status[0] & CASSETTE_SIZE_BITS) == 0x03) && TapeWidth_ == LabelManagerDriver::tw24mm))
                    return true;
                else
                    return false;
            }
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 450"))
        {
            if(((Status[1] & 0xFF) == 0x00) ||
               (((Status[1] & 0xFF) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
               (((Status[1] & 0xFF) == 0x02) && TapeWidth_ == LabelManagerDriver::tw9mm) ||
               (((Status[1] & 0xFF) == 0x03) && TapeWidth_ == LabelManagerDriver::tw12mm) ||
               (((Status[1] & 0xFF) == 0x04) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
               (((Status[1] & 0xFF) == 0x05) && TapeWidth_ == LabelManagerDriver::tw24mm))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelPOINT 350"))
        {
            if((((Status[0] & CASSETTE_SIZE_BITS) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
               (((Status[0] & CASSETTE_SIZE_BITS) == 0x02) && (TapeWidth_ == LabelManagerDriver::tw19mm || TapeWidth_ == LabelManagerDriver::tw24mm)) ||
               (((Status[0] & CASSETTE_SIZE_BITS) == 0x03) && (TapeWidth_ == LabelManagerDriver::tw9mm || TapeWidth_ == LabelManagerDriver::tw12mm)))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PnP") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 420P") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelManager 500TS"))
        {
            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if(((Status[1] & 0xFF) == 0x00) ||
                   (((Status[1] & 0xFF) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
                   (((Status[1] & 0xFF) == 0x02) && TapeWidth_ == LabelManagerDriver::tw9mm) ||
                   (((Status[1] & 0xFF) == 0x03) && TapeWidth_ == LabelManagerDriver::tw12mm) ||
                   (((Status[1] & 0xFF) == 0x04) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
                   (((Status[1] & 0xFF) == 0x05) && TapeWidth_ == LabelManagerDriver::tw24mm))
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
    LabelManagerLanguageMonitor::ProcessData(const buffer_t& Data)
    {
        PageData_.insert(PageData_.end(), Data.begin(), Data.end());
    }

    void
    LabelManagerLanguageMonitor::SetDeviceName(const std::string& Value)
    {
        DeviceName_ = Value;
    }

    void
    LabelManagerLanguageMonitor::SetTapeWidth(LabelManagerDriver::tape_width_t Value)
    {
        TapeWidth_ = Value;
    }
}; // namespace
