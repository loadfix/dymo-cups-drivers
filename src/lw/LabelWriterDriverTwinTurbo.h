#ifndef LABELWRITER_DRIVER_TWIN_TURBO_H
#define LABELWRITER_DRIVER_TWIN_TURBO_H

#include "LabelWriterDriver400.h"

namespace DymoPrinterDriver
{
// Driver for the printers supported LabelWriter TwinTurbo command set
class LabelWriterDriverTwinTurbo : public LabelWriterDriver400
{
public:
    typedef enum
    {
        ROLL_AUTO = 0,
        ROLL_LEFT,
        ROLL_RIGHT
    } roll_t;

    LabelWriterDriverTwinTurbo(IPrintEnvironment& environment);
    virtual ~LabelWriterDriverTwinTurbo() {}

    virtual void startDoc();

    roll_t getRoll();
    void   setRoll(roll_t value);

    static buffer_t getRollSelectCommand(roll_t value);

protected:
    void sendRollSelect(roll_t value);

private:
    roll_t roll;
};

}

#endif // LABELWRITER_DRIVER_TWIN_TURBO_H
