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
    environment(environment), isFirstPage(true), pageData(), useSleep(use_sleep), lastReadStatusResult(true), readStatusTimeout(read_status_timeout)
    {
    }

    LabelManagerLanguageMonitor::~LabelManagerLanguageMonitor()
    {
    }

    void
    LabelManagerLanguageMonitor::startDoc()
    {
        isFirstPage = true;
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
        if (isFirstPage)
        {
            checkStatus();
        }

        isFirstPage = false;
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

            buffer_t status;
            time_t   beginTime   = time(NULL);
            bool     statusOK    = readStatus(status);

            // request status while good or bad condition or timeout
            int i = 0;
            while ((!statusOK || (status[0] & BUSY_BIT)) && (difftime(time(NULL), beginTime) < readStatusTimeout))
            {
                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() 2 %i\n", i);

                statusOK = readStatus(status);

                i++;
            }

            if (difftime(time(NULL), beginTime) >= readStatusTimeout)
            {
                setJobStatus(BUSY_BIT);

                fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() timeout\n");

                break;
            }

            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT && !checkTapeSize(status))
                status[0] = INCORRECT_SIZE_BIT;

            // FIXME: Is the following logic correct?
            if ((status[0] == INCORRECT_SIZE_BIT) ||
                (status[0] & GENERAL_ERROR_BIT) ||
                (status[0] & HEAD_OVERHEAT_BIT) ||
                (status[0] & SLOT_STATUS_BIT) ||
                ((status[0] & CASSETTE_PRESENT_BIT) == 0))
            {
                setJobStatus(status[0]);
            }
            else
            {
                // restore good status of the job

                setJobStatus(CASSETTE_PRESENT_BIT);

                break;
            }
        }

        // clear stored label data
        pageData.clear();

        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::checkStatus() return\n");
    }

    bool
    LabelManagerLanguageMonitor::readStatus(buffer_t& status)
    {
        time_t t = time(NULL);
        fprintf(stderr, "DEBUG: LabelManagerLanguageMonitor::readStatus() %s\n", ctime(&t));

        bool result = false;
        status.clear();

        buffer_t requestStatusCommand = LabelManagerDriver::getRequestStatusCommand();
        environment.writeData(requestStatusCommand);

        environment.readData(status);

        if (status.size() > 0)
        {
            result = true;
        }

        fprintf(stderr, "DEBUG: readStatus() returned %i %i\n", (int)status.size(), (int)result);
        return result;
    }

    void
    LabelManagerLanguageMonitor::setJobStatus(byte status)
    {
        IPrintEnvironment::job_status_t jobStatus = IPrintEnvironment::jsOK;

        if (status == INCORRECT_SIZE_BIT)
            jobStatus = IPrintEnvironment::jsPaperSizeError;
        else if (status & GENERAL_ERROR_BIT)
            jobStatus = IPrintEnvironment::jsError;
        else if (status & HEAD_OVERHEAT_BIT)
            jobStatus = IPrintEnvironment::jsHeadOverheat;
        else if (status & SLOT_STATUS_BIT)
            jobStatus = IPrintEnvironment::jsSlotStatusError;
        else if (status & BUSY_BIT)
            jobStatus = IPrintEnvironment::jsBusy;
        else if ((status & CASSETTE_PRESENT_BIT) == 0)
            jobStatus = IPrintEnvironment::jsPaperSizeUndefinedError;

        environment.setJobStatus(jobStatus);
    }

    bool
    LabelManagerLanguageMonitor::checkTapeSize(buffer_t status)
    {
        fprintf(stderr, "DEBUG: checkTapeSize() device %s tape %d\n", deviceName.c_str(), tapeWidth);

        if (!strcasecmp(deviceName.c_str(), "DYMO LabelWriter DUO Tape 128") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelWriter 450 DUO Tape") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER 400") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER PC II") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER PC"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
                return true;
            else
                return false;
        }

        if (!strcasecmp(deviceName.c_str(), "DYMO LabelWriter DUO Tape"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if((((status[0] & CASSETTE_SIZE_BITS) == 0x00) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_6MM) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x01) && (tapeWidth == LabelManagerDriver::TAPE_WIDTH_9MM || tapeWidth == LabelManagerDriver::TAPE_WIDTH_12MM)) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x02) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_19MM) ||
                   (((status[0] & CASSETTE_SIZE_BITS) == 0x03) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_24MM))
                    return true;
                else
                    return false;
            }
            else
                return false;
        }

        if (!strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER 450"))
        {
            if(((status[1] & 0xFF) == 0x00) ||
               (((status[1] & 0xFF) == 0x01) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_6MM) ||
               (((status[1] & 0xFF) == 0x02) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_9MM) ||
               (((status[1] & 0xFF) == 0x03) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_12MM) ||
               (((status[1] & 0xFF) == 0x04) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_19MM) ||
               (((status[1] & 0xFF) == 0x05) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_24MM))
                return true;
            else
                return false;
        }

        if (!strcasecmp(deviceName.c_str(), "DYMO LabelPOINT 350"))
        {
            if((((status[0] & CASSETTE_SIZE_BITS) == 0x01) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_6MM) ||
               (((status[0] & CASSETTE_SIZE_BITS) == 0x02) && (tapeWidth == LabelManagerDriver::TAPE_WIDTH_19MM || tapeWidth == LabelManagerDriver::TAPE_WIDTH_24MM)) ||
               (((status[0] & CASSETTE_SIZE_BITS) == 0x03) && (tapeWidth == LabelManagerDriver::TAPE_WIDTH_9MM || tapeWidth == LabelManagerDriver::TAPE_WIDTH_12MM)))
                return true;
            else
                return false;
        }

        if (!strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER PnP") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelMANAGER 420P") ||
            !strcasecmp(deviceName.c_str(), "DYMO LabelManager 500TS"))
        {
            if((status[0] & CASSETTE_PRESENT_BIT) == CASSETTE_PRESENT_BIT)
            {
                if(((status[1] & 0xFF) == 0x00) ||
                   (((status[1] & 0xFF) == 0x01) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_6MM) ||
                   (((status[1] & 0xFF) == 0x02) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_9MM) ||
                   (((status[1] & 0xFF) == 0x03) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_12MM) ||
                   (((status[1] & 0xFF) == 0x04) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_19MM) ||
                   (((status[1] & 0xFF) == 0x05) && tapeWidth == LabelManagerDriver::TAPE_WIDTH_24MM))
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
        pageData.insert(pageData.end(), data.begin(), data.end());
    }

    void
    LabelManagerLanguageMonitor::setDeviceName(const std::string& value)
    {
        deviceName = value;
    }

    void
    LabelManagerLanguageMonitor::setTapeWidth(LabelManagerDriver::tape_width_t value)
    {
        tapeWidth = value;
    }
}; // namespace
