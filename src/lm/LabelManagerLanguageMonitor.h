#ifndef LABELMANAGER_LANGUAGE_MONITOR_H
#define LABELMANAGER_LANGUAGE_MONITOR_H

#include "PrinterDriver.h"
#include "LabelManagerDriver.h"
#include "LabelManagerDriverImpl.h"

namespace DymoPrinterDriver
{

    class LabelManagerLanguageMonitor: public ILanguageMonitor
    {
    public:
        enum status_bits
        {
            CASSETTE_SIZET0_BIT = 0x01,
            CASSETTE_SIZET1_BIT = 0x02,
            CASSETTE_SIZE_BITS  = (CASSETTE_SIZET0_BIT | CASSETTE_SIZET1_BIT),
            GENERAL_ERROR_BIT   = 0x04,
            HEAD_OVERHEAT_BIT   = 0x08,
            SLOT_STATUS_BIT     = 0x10,
            BUSY_BIT            = 0x20,
            CASSETTE_PRESENT_BIT= 0x40,
            AUTO_CUTTER_BIT     = 0x80,
            NO_POWER_BIT        = 0x80,
            INCORRECT_SIZE_BIT  = 0xFF
        };

        LabelManagerLanguageMonitor(IPrintEnvironment& environment, bool use_sleep = true, size_t read_status_timeout = 10);
        virtual ~LabelManagerLanguageMonitor();

        virtual void startDoc();
        virtual void endDoc();

        virtual void startPage();
        virtual void endPage();

        virtual void processData(const buffer_t& data);

        void setDeviceName(const std::string& value);
        void setTapeWidth(LabelManagerDriver::tape_width_t value);

    protected:
    private:
        // check if printer is locally connected, i.e. to usb port
        bool isLocal();

        // checks status
        void checkStatus();

        // Read status byte from the printer
        // return true if status has been read, false otherwise
        bool readStatus(buffer_t& status);

        // update job status based on status read from the printer
        void setJobStatus(byte status);
        bool checkTapeSize(buffer_t status);

        IPrintEnvironment&                      environment;
        bool                                    isFirstPage;
        buffer_t                                pageData;

        std::string                             deviceName;
        LabelManagerDriver::tape_width_t        tapeWidth;

        bool                                    useSleep; // for test purpose only
        bool                                    lastReadStatusResult;

        size_t                                  readStatusTimeout;
    };


}; //namespace

#endif // LABELMANAGER_LANGUAGE_MONITOR_H
