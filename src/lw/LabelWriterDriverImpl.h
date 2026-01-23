#ifndef LABELWRITER_DRIVER_IMPL_H
#define LABELWRITER_DRIVER_IMPL_H

#include "LabelWriterDriver.h"
#include "PrinterDriver.h"
#include <string>

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelWriter command set
class LabelWriterDriver : public ILabelWriterDriver
{
public:
    LabelWriterDriver(IPrintEnvironment& environment);
    virtual ~LabelWriterDriver() {}

    virtual void StartDoc();
    virtual void EndDoc();

    virtual void StartPage();
    virtual void EndPage();

    virtual void ProcessRasterLine(const buffer_t& line_buffer);

    // Device Name
    void SetDeviceName(const std::string& value) { _deviceName = value; }
    const std::string& GetDeviceName() const { return _deviceName; }

    // Label height
    virtual void SetVerticalResolution(const dword value) { _dwVerticalResolution = value; }
    virtual dword GetVerticalResolution() const { return _dwVerticalResolution; }

    // Label width
    virtual void SetHorizontalResolution(const dword value) { _dwHorizontalResolution = value; }
    virtual dword GetHorizontalResolution() const { return _dwHorizontalResolution; }

    // Max printable width
    void SetMaxPrintableWidth(const dword value) { _dwMaxPrintableWidth = value; }
    dword GetMaxPrintableWidth() const { return _dwMaxPrintableWidth; }

    // Print density
    void SetDensity (const density_t value) { _density = value; }
    density_t GetDensity() const { return _density; }

    // Print quality
    void SetQuality (const quality_t value) { _quality = value; }
    quality_t GetQuality() const { return _quality; }

    // Print speed
    void SetSpeed (const speed_t value) { _speed = value; }
    speed_t GetSpeed() const { return _speed; }

    // Support high speed printing
    void SetSupportHighSpeed (bool value) { _support_high_speed = value; }
    bool GetSupportHighSpeed() { return _support_high_speed; }

    // Paper type
    void SetPaperType (const paper_type_t value) { _paperType = value; }
    paper_type_t GetPaperType() const { return _paperType; }

    // Media type
    void SetMediaType (const media_type_t value) { _mediaType = value; }
    media_type_t GetMediaType() const { return _mediaType; }

    static bool IsBlank(const buffer_t& buf);
    static buffer_t GetResetCommand();
    static buffer_t GetRequestStatusCommand();

protected:
    // helper function
    virtual void SetStartPrintJob(const dword dw_job_id);
    virtual void SetEndPrintJob();
    virtual void SetLabelIndex(const dword dw_page_number);
    virtual void SetPrintDataHeader(const dword dw_vertical_resolution, const dword dw_horizontal_resolution);
    virtual void SetFormFeed();
    virtual void SetShortFormFeed();

    virtual void SetPrintDensity();
    virtual void SetPrintQuality();
    virtual void SetPrintSpeed();
    virtual void SetPrintMedia();
    virtual void SetLabelLength(const dword dw_length);

    virtual void ProcessRasterLineInternal(const buffer_t& line_buffer);

    virtual void SendCommand(const buffer_t& cmd_buffer);

private:
    IPrintEnvironment& _printEnvironment;

    enum { MAX_PRINTABLE_WIDTH = 672 }; // Print head width

    // job internal variables
    dword _dwVerticalResolution;
    dword _dwHorizontalResolution;

    // job params
protected:
    dword _dwPageNumber;
private:
    dword _dwJobID;

   // device params
    std::string _deviceName;
    dword _dwHeight; // Vertical resolution / height in dots
    dword _dwMaxPrintableWidth; // Printabel width in dots
    density_t _density;
    quality_t _quality;
    speed_t _speed;
    paper_type_t _paperType;
    media_type_t _mediaType;

    bool _support_high_speed;
};

}

#endif // LABELWRITER_DRIVER_IMPL_H
