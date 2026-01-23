#ifndef LABELWRITER_DRIVER_400_H
#define LABELWRITER_DRIVER_400_H

#include "LabelWriterDriverImpl.h"

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelWriter 400 series command set
class LabelWriterDriver400 : public LabelWriterDriver
{
public:
    LabelWriterDriver400(IPrintEnvironment& environment);
    virtual ~LabelWriterDriver400() {}

    virtual void startDoc();
    virtual void endDoc();
    virtual void endPage();

    static buffer_t getShortFormFeedCommand();

protected:
    void sendShortFormFeed();
};

}

#endif // LABELWRITER_DRIVER_400_H
