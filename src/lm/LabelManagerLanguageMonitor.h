#ifndef LABELMANAGER_LANGUAGE_MONITOR_H
#define LABELMANAGER_LANGUAGE_MONITOR_H

#include "PrinterDriver.h"
#include "LabelManagerDriver.h"
#include "LabelManagerDriverImpl.h"

namespace DymoPrinterDriver
{

    class CLabelManagerLanguageMonitor: public ILanguageMonitor
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

        CLabelManagerLanguageMonitor(IPrintEnvironment& Environment, bool UseSleep = true, size_t ReadStatusTimeout = 10);
        virtual ~CLabelManagerLanguageMonitor();

        virtual void StartDoc();
        virtual void EndDoc();

        virtual void StartPage();
        virtual void EndPage();

        virtual void ProcessData(const buffer_t& Data);

        void SetDeviceName(const std::string& Value);
        void SetTapeWidth(CLabelManagerDriver::tape_width_t Value);

    protected:
    private:
        // check if printer is locally connected, i.e. to usb port
        bool IsLocal();

        // checks status
        void CheckStatus();

        // Read status byte from the printer
        // return true if status has been read, false otherwise
        bool ReadStatus(buffer_t& Status);

        // update job status based on status read from the printer
        void SetJobStatus(byte Status);
        bool CheckTapeSize(buffer_t Status);

        IPrintEnvironment&                      Environment_;
        bool                                    IsFirstPage_;
        buffer_t                                PageData_;

        std::string                             DeviceName_;
        CLabelManagerDriver::tape_width_t       TapeWidth_;

        bool                                    UseSleep_; // for test purpose only
        bool                                    LastReadStatusResult_;

        size_t                                  ReadStatusTimeout_;
    };


}; //namespace

#endif // LABELMANAGER_LANGUAGE_MONITOR_H
