#include "LabelWriterDriverTwinTurbo.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

CLabelWriterDriverTwinTurbo::CLabelWriterDriverTwinTurbo(IPrintEnvironment& Environment) :
   CLabelWriterDriver400(Environment), _roll(rtAuto)
{
}

void CLabelWriterDriverTwinTurbo::StartDoc()
{
   CLabelWriterDriver400::StartDoc();
   SendRollSelect(_roll);
}

CLabelWriterDriverTwinTurbo::roll_t CLabelWriterDriverTwinTurbo::GetRoll()
{
   return _roll;
}

void CLabelWriterDriverTwinTurbo::SetRoll(CLabelWriterDriverTwinTurbo::roll_t Value)
{
   _roll = Value;
}

buffer_t CLabelWriterDriverTwinTurbo::GetRollSelectCommand(roll_t Value)
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

void CLabelWriterDriverTwinTurbo::SendRollSelect(CLabelWriterDriverTwinTurbo::roll_t Value)
{
   buffer_t buf = GetRollSelectCommand(Value);
   SendCommand(buf);
}

};
