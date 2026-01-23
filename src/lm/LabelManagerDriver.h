#ifndef LABELMANAGER_DRIVER_H
#define LABELMANAGER_DRIVER_H

#include "PrinterDriver.h"
#include <string>

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelManager command set
class ILabelManagerDriver : virtual public IPrinterDriver
{
public:
   typedef enum
   {
      tw6mm = 0,
      tw9mm,
      tw12mm,
      tw19mm,
      tw24mm
   } tape_width_t;

   typedef enum
   {
      coCut = 0,
      coChainMarks,
      coDoNothing
   } cut_t;

   typedef enum
   {
      alCenter = 0,
      alLeft,
      alRight
   } alignment_t;

   virtual ~ILabelManagerDriver() {}

   virtual void startDoc() = 0;
   virtual void endDoc() = 0;

   virtual void startPage() = 0;
   virtual void endPage() = 0;

   virtual void processRasterLine(const buffer_t& line_buffer) = 0;

protected:
   // helper function
   virtual void setStartPrintJob(const dword dw_job_id) = 0;
   virtual void setEndPrintJob() = 0;
   virtual void setFormFeed() = 0;
   virtual void setShortFormFeed() = 0;

   virtual void setCutCommand() = 0;
   virtual void setCutterMark() = 0;
};

}

#endif // LABELMANAGER_DRIVER_H
