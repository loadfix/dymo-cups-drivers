#ifndef PRINTER_DRIVER_H
#define PRINTER_DRIVER_H

#include "CommonTypedefs.h"

namespace DymoPrinterDriver
{

const byte ESC = 0x1b;

// Generic interface to the driver
class IPrinterDriver
{
public:
   typedef enum
   {
      ptRegular = 0,
      ptContinuous = 1
   } paper_type_t;

   virtual ~IPrinterDriver() {}

   virtual void startDoc() = 0;
   virtual void endDoc() = 0;

   virtual void startPage() = 0;
   virtual void endPage() = 0;

   virtual void processRasterLine(const buffer_t& line_buffer) = 0;
};

// Interface to monitor sending printer data
class ILanguageMonitor
{
public:
   virtual ~ILanguageMonitor() {}

   virtual void startDoc() = 0;
   virtual void endDoc() = 0;

   virtual void startPage() = 0;
   virtual void endPage() = 0;

   virtual void processData(const buffer_t& data) = 0;
};

// Provides interface to the environment
class IPrintEnvironment
{
public:
   typedef enum
   {
      jsOK,
      jsPaperOut,
      jsError,
      jsDeleted,
      jsPaperSizeError,
      jsPaperSizeUndefinedError,
      jsHeadOverheat,
      jsSlotStatusError,
      jsCounterfeitError,
      jsBusy
   } job_status_t;

   virtual ~IPrintEnvironment() {}

   virtual bool writeData(const buffer_t& data_buffer) = 0;
   virtual bool readData(buffer_t& data_buffer) = 0;

   virtual job_status_t getJobStatus() = 0;
   virtual void setJobStatus(job_status_t job_status) = 0;
};

}; // namespace

#endif // PRINTER_DRIVER_H
