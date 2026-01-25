#ifndef LABELWRITER_DRIVER_H
#define LABELWRITER_DRIVER_H

#include <stdlib.h>
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelWriter command set
class ILabelWriterDriver : virtual public IPrinterDriver
{
public:
    typedef enum
    {
        PRINT_DENSITY_LOW = 0,
        PRINT_DENSITY_MEDIUM,
        PRINT_DENSITY_NORMAL,
        PRINT_DENSITY_HIGH
    } density_t;

    typedef enum
    {
        PRINT_QUALITY_TEXT = 0,
        PRINT_QUALITY_BARCODE_AND_GRAPHICS
    } quality_t;

    typedef enum
    {
        MEDIA_TYPE_DEFAULT = 0,
        MEDIA_TYPE_DURABLE
    } media_type_t;

    typedef enum
    {
        PRINT_SPEED_NORMAL = 0,
        PRINT_SPEED_HIGH
    } speed_t;

    virtual ~ILabelWriterDriver() {}

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
};

}

#endif // LABELWRITER_DRIVER_H
