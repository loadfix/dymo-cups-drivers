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

   virtual void StartDoc() = 0;
   virtual void EndDoc() = 0;

   virtual void StartPage() = 0;
   virtual void EndPage() = 0;

   virtual void ProcessRasterLine(const buffer_t& line_buffer) = 0;
};

// Interface to monitor sending printer data
class ILanguageMonitor
{
public:
   virtual ~ILanguageMonitor() {}

   virtual void StartDoc() = 0;
   virtual void EndDoc() = 0;

   virtual void StartPage() = 0;
   virtual void EndPage() = 0;

   virtual void ProcessData(const buffer_t& data) = 0;
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

   virtual bool WriteData(const buffer_t& data_buffer) = 0;
   virtual bool ReadData(buffer_t& data_buffer) = 0;

   virtual job_status_t GetJobStatus() = 0;
   virtual void SetJobStatus(job_status_t job_status) = 0;
};

}; // namespace

#endif // PRINTER_DRIVER_H
