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
   void setDeviceName(const std::string& value) { _deviceName = value; }
   const std::string& getDeviceName() const { return _deviceName; }

   // Auto Cut Support
   virtual void setSupportAutoCut(const bool value) { _supportAutoCut = value; }
   virtual bool getSupportAutoCut() const { return _supportAutoCut; }

   // Cut, cutter marks or nothing
   virtual void setCutOption(const ILabelManagerDriver::cut_t value) { _cutOptions = value; fprintf(stderr, "TEST: setCutOption = %d\n", value); }
   virtual ILabelManagerDriver::cut_t getCutOption() const { return _cutOptions; }

   // Label alignment on the tape
   virtual void setAlignment(const ILabelManagerDriver::alignment_t value) { _alignment = value; fprintf(stderr, "TEST: setAlignment = %d\n", value); }
   virtual ILabelManagerDriver::alignment_t getAlignment() const { return _alignment; }

   // Label height
   virtual void setVerticalResolution(const dword value) { _dwVerticalResolution = value; }
   virtual dword getVerticalResolution() const { return _dwVerticalResolution; }

   // Label width
   virtual void setHorizontalResolution(const dword value) { _dwHorizontalResolution = value; }
   virtual dword getHorizontalResolution() const { return _dwHorizontalResolution; }

   void setPrintChainMarksAtDocEnd(const bool value) { _printChainMarksAtDocEnd = value; fprintf(stderr, "TEST: setPrintChainMarksAtDocEnd = %d\n", value); }
   bool getPrintChainMarksAtDocEnd() const { return _printChainMarksAtDocEnd; }

   // Min label length in dots at printer DPI (300dpi -> 300 dots for 1 inch)
   void setMinLabelLength(const dword value) { _dwMinPageLine = value; }
   dword getMinLabelLength() const { return _dwMinPageLine; }

   // Tape alignment offset
   void setTapeAlignmentOffset(const dword value) { _dwTapeAlignmentOffset = value; fprintf(stderr, "INFORMATION - Setting shift value: %d\n", _dwTapeAlignmentOffset); }
   dword getTapeAlignmentOffset() const { return _dwTapeAlignmentOffset; }

   // Normal leader - no padding required since FW prints center aligned
   void setNormalLeader(const dword value) { _dwNormalLeader = value; }
   dword getNormalLeader() const { return _dwNormalLeader; }

   // Min leader - no padding required since FW prints center aligned
   void setMinLeader(const dword value) { _dwMinLeader = value; }
   dword getMinLeader() const { return _dwMinLeader; }

   // Aligned leader - no padding required since FW prints center aligned
   void setAlignedLeader(const dword value) { _dwAlignedLeader = value; }
   dword getAlignedLeader() const { return _dwAlignedLeader; }

   // Max printable width
   void setMaxPrintableWidth(const dword value) { _dwMaxPrintableWidth = value; }
   dword getMaxPrintableWidth() const { return _dwMaxPrintableWidth; }

   // Paper type
   void setPaperType (const paper_type_t value) { _paperType = value; }
   paper_type_t getPaperType() const { return _paperType; }

   static bool isBlank(const buffer_t& buf) { return false; }
   static buffer_t getRequestStatusCommand();

protected:
   // helper function
   virtual void setStartPrintJob(const dword dw_job_id);
   virtual void setEndPrintJob();
   virtual void setLabelIndex(const dword dw_page_number);
   virtual void setLabelLeader(const dword dw_length);
   virtual void setLabelTrailer(const dword dw_length);
   virtual void setPrintDataHeader(const dword dw_vertical_resolution, const dword dw_horizontal_resolution);
   virtual void setFormFeed();
   virtual void setShortFormFeed();

   virtual void setCutCommand();
   virtual void setCutterMark();

   virtual void processRasterLineInternal(const buffer_t& line_buffer);

   virtual void shiftData(const buffer_t& buf, buffer_t& shifted_buf, int shift_value);
   virtual void shiftDataLeft(const buffer_t& buf, buffer_t& shifted_buf, size_t shift_value);
   virtual void shiftDataRight(const buffer_t& buf, buffer_t& shifted_buf, size_t shift_value);
   virtual int getShiftValue(size_t raster_line_size);

   virtual void sendCommand(const buffer_t& cmd_buffer);

private:
   IPrintEnvironment& _printEnvironment;

   enum { MIN_LABEL_LENGTH = 210 }; // Min label is 40mm - 11mm(leader) - 11mm(trailer) = 18mm -> ~ 210 dots/lines
   enum { MAX_PRINTABLE_WIDTH = 256 }; // Print head width
   enum { NORMAL_LEADER = 130 }; // Normal leader is 11mm
   enum { MIN_LEADER = 94 }; // Normal leader - print line and cutter distance which is about 8mm
   enum { MIN_ALIGNED_LEADER = 28 }; // Sync leader and trailer

   // job internal variables
   dword _dwVerticalResolution;
   dword _dwHorizontalResolution;

   // job params
   bool _jobDidStart;
   dword _dwPageNumber;
   dword _dwJobID;
   ILabelManagerDriver::cut_t _cutOptions;
   ILabelManagerDriver::alignment_t _alignment; // not being used since it will be always center aligned
   dword _dwTapeAlignmentOffset;  // offset to justify output for the current label type. It is different for each tape size and model
   paper_type_t _paperType;

   // device params
   std::string _deviceName;
   bool _supportAutoCut;
   bool _printChainMarksAtDocEnd;

   dword _dwMinPageLine; // Min label
   dword _dwHeight; // Vertical resolution / height in dots
   dword _dwMaxPrintableWidth; // Printabel width in dots
   dword _dwNormalLeader; // Normal leader in dots
   dword _dwMinLeader; // Min leader in dots
   dword _dwAlignedLeader; // Aligned leader in dots
};

}

#endif // LABELMANAGER_DRIVER_IMPL_H
