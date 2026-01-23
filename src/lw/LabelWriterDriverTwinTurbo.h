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
        rtAuto = 0,
        rtLeft,
        rtRight
    } roll_t;

    LabelWriterDriverTwinTurbo(IPrintEnvironment& environment);
    virtual ~LabelWriterDriverTwinTurbo() {}

    virtual void StartDoc();

    roll_t GetRoll();
    void   SetRoll(roll_t value);

    static buffer_t GetRollSelectCommand(roll_t value);

protected:
    void SendRollSelect(roll_t value);

private:
    roll_t _roll;
};

}

#endif // LABELWRITER_DRIVER_TWIN_TURBO_H
