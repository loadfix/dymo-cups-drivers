#include "LabelManagerLanguageMonitor.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace DymoPrinterDriver
{
    LabelManagerLanguageMonitor::LabelManagerLanguageMonitor(IPrintEnvironment& environment, bool use_sleep, size_t read_status_timeout):
    Environment_(environment), IsFirstPage_(true), PageData_(), UseSleep_(use_sleep), LastReadStatusResult_(true), ReadStatusTimeout_(read_status_timeout)
    {
    }

    LabelManagerLanguageMonitor::~LabelManagerLanguageMonitor()
    {
    }

    void
    LabelManagerLanguageMonitor::startDoc()
    {
        IsFirstPage_ = true;
    }

    void
    LabelManagerLanguageMonitor::endDoc()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::endDoc()\n");
    }

    void
    LabelManagerLanguageMonitor::startPage()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::startPage()\n");
        if (IsFirstPage_)
        {
            checkStatus();
        }

        IsFirstPage_ = false;
    }

    void
    LabelManagerLanguageMonitor::endPage()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::endPage()\n");
    }

    bool
    LabelManagerLanguageMonitor::isLocal()
    {
        char* uri = getenv("DEVICE_URI");

        return (strncmp(uri, "usb://", 6) == 0);
    }

    void
    LabelManagerLanguageMonitor::checkStatus()
    {
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus()\n");

        // restore good status of the job

        setJobStatus(CASSETTE_PRESENT_BIT);

        if(!isLocal())
            return;

        while (true)
        {
            fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() 1\n");

            buffer_t Status;
            time_t   BeginTime   = time(NULL);
            bool     StatusOK    = readStatus(Status);

            // request status while good or bad condition or timeout
            int i = 0;
            while ((!StatusOK || (Status[0] & BUSY_BIT)) && (difftime(time(NULL), BeginTime) < ReadStatusTimeout_))
            {
                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() 2 %i\n", i);

                StatusOK = readStatus(Status);

                i++;
            }

            if (difftime(time(NULL), BeginTime) >= ReadStatusTimeout_)
            {
                setJobStatus(BUSY_BIT);

                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() timeout\n");

                break;
            }

            if((Status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT && !checkTapeSize(Status))
                Status[0] = INCORRECT_SIZE_BIT;

            if ((Status[0] == INCORRECT_SIZE_BIT) ||
                (Status[0] & GENERAL_ERROR_BIT) ||
                (Status[0] & HEAD_OVERHEAT_BIT) ||
                (Status[0] & SLOT_STATUS_BIT) ||
                ((Status[0] & CASSETTE_PRESENT_BIT) == 0))
            {
                setJobStatus(Status[0]);
            }
            else
            {
                // restore good status of the job

                setJobStatus(CASSETTE_PRESENT_BIT);

                break;
            }
        }

        // clear stored label data
        PageData_.clear();

        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() return\n");
    }

    bool
    LabelManagerLanguageMonitor::readStatus(buffer_t& status)
    {
        time_t t = time(NULL);
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::readStatus() %s\n", ctime(&t));

        bool Result = false;
        status.clear();

        buffer_t RequestStatusCommand = LabelManagerDriver::getRequestStatusCommand();
        Environment_.writeData(RequestStatusCommand);

        Environment_.readData(status);

        if (status.size() > 0)
        {
            Result = true;
        }

        fprintf(stderr, "DEBUG: readStatus() returned %i %i\n", (int)status.size(), (int)Result);
        return Result;
    }

    void
    LabelManagerLanguageMonitor::setJobStatus(byte status)
    {
        IPrintEnvironment::job_status_t JobStatus = IPrintEnvironment::jsOK;

        if (status == INCORRECT_SIZE_BIT)
            JobStatus = IPrintEnvironment::jsPaperSizeError;
        else if (status & GENERAL_ERROR_BIT)
            JobStatus = IPrintEnvironment::jsError;
        else if (status & HEAD_OVERHEAT_BIT)
            JobStatus = IPrintEnvironment::jsHeadOverheat;
        else if (status & SLOT_STATUS_BIT)
            JobStatus = IPrintEnvironment::jsSlotStatusError;
        else if (status & BUSY_BIT)
            JobStatus = IPrintEnvironment::jsBusy;
        else if ((status & CASSETTE_PRESENT_BIT) == 0)
            JobStatus = IPrintEnvironment::jsPaperSizeUndefinedError;

        Environment_.setJobStatus(JobStatus);
    }

    bool
    LabelManagerLanguageMonitor::checkTapeSize(buffer_t status)
    {
        fprintf(stderr, "DEBUG: checkTapeSize() device %s tape %d\n", DeviceName_.c_str(), TapeWidth_);

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter DUO Tape 128") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter 450 DUO Tape") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 400") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PC II") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PC"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelWriter DUO Tape"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if((((status[0] & CASSETTE_SIZE_BITS) == 0x00) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x01) && (TapeWidth_ == LabelManagerDriver::tw9mm || TapeWidth_ == LabelManagerDriver::tw12mm)) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x02) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x03) && TapeWidth_ == LabelManagerDriver::tw24mm))
                    return true;
                else
                    return false;
            }
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 450"))
        {
            if(((status[1] & 0xFF) == 0x00) ||
               (((status[1] & 0xFF) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
               (((status[1] & 0xFF) == 0x02) && TapeWidth_ == LabelManagerDriver::tw9mm) ||
               (((status[1] & 0xFF) == 0x03) && TapeWidth_ == LabelManagerDriver::tw12mm) ||
               (((status[1] & 0xFF) == 0x04) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
               (((status[1] & 0xFF) == 0x05) && TapeWidth_ == LabelManagerDriver::tw24mm))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelPOINT 350"))
        {
            if((((status[0] & CASSETTE_SIZE_BITS) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
               (((status[0] & CASSETTE_SIZE_BITS) == 0x02) && (TapeWidth_ == LabelManagerDriver::tw19mm || TapeWidth_ == LabelManagerDriver::tw24mm)) ||
               (((status[0] & CASSETTE_SIZE_BITS) == 0x03) && (TapeWidth_ == LabelManagerDriver::tw9mm || TapeWidth_ == LabelManagerDriver::tw12mm)))
                return true;
            else
                return false;
        }

        if (!strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER PnP") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelMANAGER 420P") ||
            !strcasecmp(DeviceName_.c_str(), "DYMO LabelManager 500TS"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if(((status[1] & 0xFF) == 0x00) ||
                   (((status[1] & 0xFF) == 0x01) && TapeWidth_ == LabelManagerDriver::tw6mm) ||
                   (((status[1] & 0xFF) == 0x02) && TapeWidth_ == LabelManagerDriver::tw9mm) ||
                   (((status[1] & 0xFF) == 0x03) && TapeWidth_ == LabelManagerDriver::tw12mm) ||
                   (((status[1] & 0xFF) == 0x04) && TapeWidth_ == LabelManagerDriver::tw19mm) ||
                   (((status[1] & 0xFF) == 0x05) && TapeWidth_ == LabelManagerDriver::tw24mm))
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
    LabelManagerLanguageMonitor::processData(const buffer_t& data)
    {
        PageData_.insert(PageData_.end(), data.begin(), data.end());
    }

    void
    LabelManagerLanguageMonitor::setDeviceName(const std::string& value)
    {
        DeviceName_ = value;
    }

    void
    LabelManagerLanguageMonitor::setTapeWidth(LabelManagerDriver::tape_width_t value)
    {
        TapeWidth_ = value;
    }
}; // namespace
