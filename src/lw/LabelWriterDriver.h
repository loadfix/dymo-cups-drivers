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
        pdLow = 0,
        pdMedium,
        pdNormal,
        pdHigh
    } density_t;

    typedef enum
    {
        pqText = 0,
        pqBarcodeAndGraphics
    } quality_t;

    typedef enum
    {
        mtDefault = 0,
        mtDurable
    } media_type_t;

    typedef enum
    {
        psNormal = 0,
        psHigh
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
