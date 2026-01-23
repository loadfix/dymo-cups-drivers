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

   virtual void StartDoc() = 0;
   virtual void EndDoc() = 0;

   virtual void StartPage() = 0;
   virtual void EndPage() = 0;

   virtual void ProcessRasterLine(const buffer_t& LineBuffer) = 0;

protected:
   // helper function
   virtual void SetStartPrintJob(const dword dwJobID) = 0;
   virtual void SetEndPrintJob() = 0;
   virtual void SetFormFeed() = 0;
   virtual void SetShortFormFeed() = 0;

   virtual void SetCutCommand() = 0;
   virtual void SetCutterMark() = 0;
};

}

#endif // LABELMANAGER_DRIVER_H
