#include "LabelWriterDriverTwinTurbo.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

LabelWriterDriverTwinTurbo::LabelWriterDriverTwinTurbo(IPrintEnvironment& Environment) :
   LabelWriterDriver400(Environment), _roll(rtAuto)
{
}

void LabelWriterDriverTwinTurbo::StartDoc()
{
   LabelWriterDriver400::StartDoc();
   SendRollSelect(_roll);
}

LabelWriterDriverTwinTurbo::roll_t LabelWriterDriverTwinTurbo::GetRoll()
{
   return _roll;
}

void LabelWriterDriverTwinTurbo::SetRoll(LabelWriterDriverTwinTurbo::roll_t Value)
{
   _roll = Value;
}

buffer_t LabelWriterDriverTwinTurbo::GetRollSelectCommand(roll_t Value)
{
   byte buf[] = {ESC, 'q', '0'};

   switch (Value)
   {
      case rtLeft:    buf[2] = '1'; break;
      case rtRight:   buf[2] = '2'; break;
      default:        buf[2] = '0'; break;
   }

   return buffer_t(buf, buf + sizeof(buf));
}

void LabelWriterDriverTwinTurbo::SendRollSelect(LabelWriterDriverTwinTurbo::roll_t Value)
{
   buffer_t buf = GetRollSelectCommand(Value);
   SendCommand(buf);
}

};
