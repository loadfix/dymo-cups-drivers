#ifndef LABELWRITER_LANGUAGE_MONITOR_H
#define LABELWRITER_LANGUAGE_MONITOR_H

#include "PrinterDriver.h"
#include "LabelWriterDriver.h"
#include "LabelWriterDriverImpl.h"
#include "LabelWriterDriverTwinTurbo.h"

namespace DymoPrinterDriver
{



class LabelWriterLanguageMonitor: public ILanguageMonitor
{
public:
  enum status_bits
  {
    TOF_BIT             = 0x02,
    ROLL_CHANGED_BIT    = 0x08,
    PAPER_OUT_BIT       = 0x20,
    PAPER_FEED_BIT      = 0x40,
    ERROR_BIT           = 0x80,
  };

  LabelWriterLanguageMonitor(IPrintEnvironment& Environment, bool UseSleep = true, size_t ReadStatusTimeout = 10);
  virtual ~LabelWriterLanguageMonitor();

  virtual void StartDoc();
  virtual void EndDoc();

  virtual void StartPage();
  virtual void EndPage();

  virtual void ProcessData(const buffer_t& Data);

  // some values used by driver is also interesting for the language monitor
  void SetPaperType(LabelWriterDriver::paper_type_t  Value);
  void SetRoll(LabelWriterDriverTwinTurbo::roll_t    Value);


protected:
private:
  // check if printer is locally connected, i.e. to usb port
  bool IsLocal();

  // send reset command to reset from probably nonproper finished previous job
  void ResetPrinter();

  // send ESC q as first command to synchronize roll used by drivers and the device
  // it is needed to properly read status byte from the active roll
  void SynchronizeRoll();

  // checks status and reprint label if needed
  void CheckStatusAndReprint();

  // Read status byte from the printer
  // return true if status has been read, false otherwise
  bool ReadStatus(byte& Status);

  // request status while the status become OK
  // return true on success, false - otherwise
  bool PollUntilPaperIn();

  // update job status based on status read from the printer
  void SetJobStatus(byte Status);

  // reprint cached label
  void ReprintLabel();

  IPrintEnvironment&                      Environment_;

  LabelWriterDriver::paper_type_t        PaperType_;
  LabelWriterDriverTwinTurbo::roll_t     Roll_;
  bool                                    RollUsed_;
  bool                                    IsFirstPage_;

  buffer_t                                PageData_;

  bool                                    UseSleep_; // for test purpose only
  byte                                    LastStatus_;
  bool                                    LastReadStatusResult_;

  size_t                                  ReadStatusTimeout_;
};


}; //namespace

#endif // LABELWRITER_LANGUAGE_MONITOR_H
