#ifndef LABELMANAGER_DRIVER_IMPL_H
#define LABELMANAGER_DRIVER_IMPL_H

#include "LabelManagerDriver.h"
#include "PrinterDriver.h"
#include <string>

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelManager command set
class LabelManagerDriver : public ILabelManagerDriver
{
public:
   LabelManagerDriver(IPrintEnvironment& environment);
   virtual ~LabelManagerDriver() {}

   virtual void startDoc();
   virtual void endDoc();

   virtual void startPage();
   virtual void endPage();

   virtual void processRasterLine(const buffer_t& line_buffer);

   // Device Name
   void setDeviceName(const std::string& value) { deviceName = value; }
   const std::string& getDeviceName() const { return deviceName; }

   // Auto Cut Support
   virtual void setSupportAutoCut(const bool value) { supportAutoCut = value; }
   virtual bool getSupportAutoCut() const { return supportAutoCut; }

   // Cut, cutter marks or nothing
   virtual void setCutOption(const ILabelManagerDriver::cut_option_t value) { cutOptions = value; fprintf(stderr, "TEST: setCutOption = %d\n", value); }
   virtual ILabelManagerDriver::cut_option_t getCutOption() const { return cutOptions; }

   // Label alignment on the tape
   virtual void setAlignment(const ILabelManagerDriver::alignment_t value) { alignment = value; fprintf(stderr, "TEST: setAlignment = %d\n", value); }
   virtual ILabelManagerDriver::alignment_t getAlignment() const { return alignment; }

   // Label height
   virtual void setVerticalResolution(const dword value) { verticalResolution = value; }
   virtual dword getVerticalResolution() const { return verticalResolution; }

   // Label width
   virtual void setHorizontalResolution(const dword value) { horizontalResolution = value; }
   virtual dword getHorizontalResolution() const { return horizontalResolution; }

   void setPrintChainMarksAtDocEnd(const bool value) { printChainMarksAtDocEnd = value; fprintf(stderr, "TEST: setPrintChainMarksAtDocEnd = %d\n", value); }
   bool getPrintChainMarksAtDocEnd() const { return printChainMarksAtDocEnd; }

   // Min label length in dots at printer DPI (300dpi -> 300 dots for 1 inch)
   void setMinLabelLength(const dword value) { minPageLine = value; }
   dword getMinLabelLength() const { return minPageLine; }

   // Tape alignment offset
   void setTapeAlignmentOffset(const dword value) { tapeAlignmentOffset = value; fprintf(stderr, "INFORMATION - Setting shift value: %d\n", tapeAlignmentOffset); }
   dword getTapeAlignmentOffset() const { return tapeAlignmentOffset; }

   // Normal leader - no padding required since FW prints center aligned
   void setNormalLeader(const dword value) { normalLeader = value; }
   dword getNormalLeader() const { return normalLeader; }

   // Min leader - no padding required since FW prints center aligned
   void setMinLeader(const dword value) { minLeader = value; }
   dword getMinLeader() const { return minLeader; }

   // Aligned leader - no padding required since FW prints center aligned
   void setAlignedLeader(const dword value) { alignedLeader = value; }
   dword getAlignedLeader() const { return alignedLeader; }

   // Max printable width
   void setMaxPrintableWidth(const dword value) { maxPrintableWidth = value; }
   dword getMaxPrintableWidth() const { return maxPrintableWidth; }

   // Paper type
   void setPaperType (const paper_type_t value) { paperType = value; }
   paper_type_t getPaperType() const { return paperType; }

   static bool isBlank(const buffer_t& buf) { return false; }
   static buffer_t getRequestStatusCommand();

protected:
   // helper function
   virtual void setStartPrintJob(const dword job_id);
   virtual void setEndPrintJob();
   virtual void setLabelIndex(const dword page_number);
   virtual void setLabelLeader(const dword length);
   virtual void setLabelTrailer(const dword length);
   virtual void setPrintDataHeader(const dword vertical_resolution, const dword horizontal_resolution);
   virtual void setFormFeed();
   virtual void setShortFormFeed();

   virtual void setCutCommand();
   virtual void setCutterMark();

   virtual void processRasterLineInternal(const buffer_t& line_buffer);

   virtual void shiftData(const buffer_t& buf, buffer_t& shifted_buf, int shift_value);
   virtual void shiftDataLeft(const buffer_t& buf, buffer_t& shifted_buf, size_t shift_value);
   virtual void shiftDataRight(const buffer_t& buf, buffer_t& shifted_buf, size_t shift_value);
   virtual int getShiftValue(size_t raster_line_size);

   virtual void sendCommand(const buffer_t& command_buffer);

private:
   IPrintEnvironment& printEnvironment;

   enum { MIN_LABEL_LENGTH = 210 }; // Min label is 40mm - 11mm(leader) - 11mm(trailer) = 18mm -> ~ 210 dots/lines
   enum { MAX_PRINTABLE_WIDTH = 256 }; // Print head width
   enum { NORMAL_LEADER = 130 }; // Normal leader is 11mm
   enum { MIN_LEADER = 94 }; // Normal leader - print line and cutter distance which is about 8mm
   enum { MIN_ALIGNED_LEADER = 28 }; // Sync leader and trailer

   // job internal variables
   dword verticalResolution;
   dword horizontalResolution;

   // job params
   bool jobDidStart;
   dword pageNumber;
   dword jobID;
   ILabelManagerDriver::cut_option_t cutOptions;
   ILabelManagerDriver::alignment_t alignment; // not being used since it will be always center aligned
   dword tapeAlignmentOffset;  // offset to justify output for the current label type. It is different for each tape size and model
   paper_type_t paperType;

   // device params
   std::string deviceName;
   bool supportAutoCut;
   bool printChainMarksAtDocEnd;

   dword minPageLine; // Min label
   dword height; // Vertical resolution / height in dots
   dword maxPrintableWidth; // Printabel width in dots
   dword normalLeader; // Normal leader in dots
   dword minLeader; // Min leader in dots
   dword alignedLeader; // Aligned leader in dots
};

}

#endif // LABELMANAGER_DRIVER_IMPL_H
