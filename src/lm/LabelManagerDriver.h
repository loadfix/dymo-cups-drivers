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
      TAPE_WIDTH_6MM = 0,
      TAPE_WIDTH_9MM,
      TAPE_WIDTH_12MM,
      TAPE_WIDTH_19MM,
      TAPE_WIDTH_24MM
   } tape_width_t;

   typedef enum
   {
      CUT_OPTION_CUT = 0,
      CUT_OPTION_CHAIN_MARKS,
      CUT_OPTION_DO_NOTHING
   } cut_option_t;

   typedef enum
   {
      ALIGN_CENTER = 0,
      ALIGN_LEFT,
      ALIGN_RIGHT
   } alignment_t;

   virtual ~ILabelManagerDriver() {}

   virtual void startDoc() = 0;
   virtual void endDoc() = 0;

   virtual void startPage() = 0;
   virtual void endPage() = 0;

   virtual void processRasterLine(const buffer_t& line_buffer) = 0;

protected:
   // helper function
   virtual void setStartPrintJob(const dword job_id) = 0;
   virtual void setEndPrintJob() = 0;
   virtual void setFormFeed() = 0;
   virtual void setShortFormFeed() = 0;

   virtual void setCutCommand() = 0;
   virtual void setCutterMark() = 0;
};

}

#endif // LABELMANAGER_DRIVER_H
