#ifndef LABELWRITER_DRIVER_400_H
#define LABELWRITER_DRIVER_400_H

#include "LabelWriterDriverImpl.h"

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelWriter 400 series command set
class CLabelWriterDriver400 : public CLabelWriterDriver
{
public:
    CLabelWriterDriver400(IPrintEnvironment& Environment);
    virtual ~CLabelWriterDriver400() {}

    virtual void StartDoc();
    virtual void EndDoc();
    virtual void EndPage();

    static buffer_t GetShortFormFeedCommand();

protected:
    void SendShortFormFeed();
};

}

#endif // LABELWRITER_DRIVER_400_H
