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

  LabelWriterLanguageMonitor(IPrintEnvironment& environment, bool use_sleep = true, size_t read_status_timeout = 10);
  virtual ~LabelWriterLanguageMonitor();

  virtual void startDoc();
  virtual void endDoc();

  virtual void startPage();
  virtual void endPage();

  virtual void processData(const buffer_t& data);

  // some values used by driver is also interesting for the language monitor
  void setPaperType(LabelWriterDriver::paper_type_t  value);
  void setRoll(LabelWriterDriverTwinTurbo::roll_t    value);


protected:
private:
  // check if printer is locally connected, i.e. to usb port
  bool isLocal();

  // send reset command to reset from probably nonproper finished previous job
  void resetPrinter();

  // send ESC q as first command to synchronize roll used by drivers and the device
  // it is needed to properly read status byte from the active roll
  void synchronizeRoll();

  // checks status and reprint label if needed
  void checkStatusAndReprint();

  // Read status byte from the printer
  // return true if status has been read, false otherwise
  bool readStatus(byte& status);

  // request status while the status become OK
  // return true on success, false - otherwise
  bool pollUntilPaperIn();

  // update job status based on status read from the printer
  void setJobStatus(byte status);

  // reprint cached label
  void reprintLabel();

  IPrintEnvironment&                      environment;

  LabelWriterDriver::paper_type_t         paperType;
  LabelWriterDriverTwinTurbo::roll_t      roll;
  bool                                    rollUsed;
  bool                                    isFirstPage;

  buffer_t                                pageData;

  bool                                    useSleep; // for test purpose only
  byte                                    lastStatus;
  bool                                    lastReadStatusResult;

  size_t                                  readStatusTimeout;
};


}; //namespace

#endif // LABELWRITER_LANGUAGE_MONITOR_H
