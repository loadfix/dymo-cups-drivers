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
};

}

#endif // LABELWRITER_DRIVER_H
