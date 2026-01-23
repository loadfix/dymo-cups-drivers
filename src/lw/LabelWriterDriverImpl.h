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
    void setDeviceName(const std::string& value) { deviceName = value; }
    const std::string& getDeviceName() const { return deviceName; }

    // Label height
    virtual void setVerticalResolution(const dword value) { verticalResolution = value; }
    virtual dword getVerticalResolution() const { return verticalResolution; }

    // Label width
    virtual void setHorizontalResolution(const dword value) { horizontalResolution = value; }
    virtual dword getHorizontalResolution() const { return horizontalResolution; }

    // Max printable width
    void setMaxPrintableWidth(const dword value) { maxPrintableWidth = value; }
    dword getMaxPrintableWidth() const { return maxPrintableWidth; }

    // Print density
    void setDensity (const density_t value) { density = value; }
    density_t getDensity() const { return density; }

    // Print quality
    void setQuality (const quality_t value) { quality = value; }
    quality_t getQuality() const { return quality; }

    // Print speed
    void setSpeed (const speed_t value) { speed = value; }
    speed_t getSpeed() const { return speed; }

    // Support high speed printing
    void setSupportHighSpeed (bool value) { supportHighSpeed = value; }
    bool getSupportHighSpeed() { return supportHighSpeed; }

    // Paper type
    void setPaperType (const paper_type_t value) { paperType = value; }
    paper_type_t getPaperType() const { return paperType; }

    // Media type
    void setMediaType (const media_type_t value) { mediaType = value; }
    media_type_t getMediaType() const { return mediaType; }

    static bool isBlank(const buffer_t& buf);
    static buffer_t getResetCommand();
    static buffer_t getRequestStatusCommand();

protected:
    // helper function
    virtual void setStartPrintJob(const dword job_id);
    virtual void setEndPrintJob();
    virtual void setLabelIndex(const dword page_number);
    virtual void setPrintDataHeader(const dword vertical_resolution, const dword horizontal_resolution);
    virtual void setFormFeed();
    virtual void setShortFormFeed();

    virtual void setPrintDensity();
    virtual void setPrintQuality();
    virtual void setPrintSpeed();
    virtual void setPrintMedia();
    virtual void setLabelLength(const dword length);

    virtual void processRasterLineInternal(const buffer_t& line_buffer);

    virtual void sendCommand(const buffer_t& command_buffer);

private:
    IPrintEnvironment& printEnvironment;

    enum { MAX_PRINTABLE_WIDTH = 672 }; // Print head width

    // job internal variables
    dword verticalResolution;
    dword horizontalResolution;

    // job params
protected:
    dword pageNumber;
private:
    dword jobID;

   // device params
    std::string deviceName;
    dword height; // Vertical resolution / height in dots
    dword maxPrintableWidth; // Printabel width in dots
    density_t density;
    quality_t quality;
    speed_t speed;
    paper_type_t paperType;
    media_type_t mediaType;

    bool supportHighSpeed;
};

}

#endif // LABELWRITER_DRIVER_IMPL_H
