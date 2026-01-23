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

    virtual void startDoc();
    virtual void endDoc();

    virtual void startPage();
    virtual void endPage();

    virtual void processRasterLine(const buffer_t& line_buffer);

    // Device Name
    void setDeviceName(const std::string& value) { _deviceName = value; }
    const std::string& getDeviceName() const { return _deviceName; }

    // Label height
    virtual void setVerticalResolution(const dword value) { _dwVerticalResolution = value; }
    virtual dword getVerticalResolution() const { return _dwVerticalResolution; }

    // Label width
    virtual void setHorizontalResolution(const dword value) { _dwHorizontalResolution = value; }
    virtual dword getHorizontalResolution() const { return _dwHorizontalResolution; }

    // Max printable width
    void setMaxPrintableWidth(const dword value) { _dwMaxPrintableWidth = value; }
    dword getMaxPrintableWidth() const { return _dwMaxPrintableWidth; }

    // Print density
    void setDensity (const density_t value) { _density = value; }
    density_t getDensity() const { return _density; }

    // Print quality
    void setQuality (const quality_t value) { _quality = value; }
    quality_t getQuality() const { return _quality; }

    // Print speed
    void setSpeed (const speed_t value) { _speed = value; }
    speed_t getSpeed() const { return _speed; }

    // Support high speed printing
    void setSupportHighSpeed (bool value) { _support_high_speed = value; }
    bool getSupportHighSpeed() { return _support_high_speed; }

    // Paper type
    void setPaperType (const paper_type_t value) { _paperType = value; }
    paper_type_t getPaperType() const { return _paperType; }

    // Media type
    void setMediaType (const media_type_t value) { _mediaType = value; }
    media_type_t getMediaType() const { return _mediaType; }

    static bool isBlank(const buffer_t& buf);
    static buffer_t getResetCommand();
    static buffer_t getRequestStatusCommand();

protected:
    // helper function
    virtual void setStartPrintJob(const dword dw_job_id);
    virtual void setEndPrintJob();
    virtual void setLabelIndex(const dword dw_page_number);
    virtual void setPrintDataHeader(const dword dw_vertical_resolution, const dword dw_horizontal_resolution);
    virtual void setFormFeed();
    virtual void setShortFormFeed();

    virtual void setPrintDensity();
    virtual void setPrintQuality();
    virtual void setPrintSpeed();
    virtual void setPrintMedia();
    virtual void setLabelLength(const dword dw_length);

    virtual void processRasterLineInternal(const buffer_t& line_buffer);

    virtual void sendCommand(const buffer_t& cmd_buffer);

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
