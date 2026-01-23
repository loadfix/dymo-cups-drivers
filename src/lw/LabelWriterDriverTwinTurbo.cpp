#include "LabelWriterDriverTwinTurbo.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

LabelWriterDriverTwinTurbo::LabelWriterDriverTwinTurbo(IPrintEnvironment& environment) :
   LabelWriterDriver400(environment), _roll(rtAuto)
{
}

void LabelWriterDriverTwinTurbo::startDoc()
{
   LabelWriterDriver400::startDoc();
   sendRollSelect(_roll);
}

LabelWriterDriverTwinTurbo::roll_t LabelWriterDriverTwinTurbo::getRoll()
{
   return _roll;
}

void LabelWriterDriverTwinTurbo::setRoll(LabelWriterDriverTwinTurbo::roll_t value)
{
   _roll = value;
}

buffer_t LabelWriterDriverTwinTurbo::getRollSelectCommand(roll_t value)
{
   byte buf[] = {ESC, 'q', '0'};

   switch (value)
   {
      case rtLeft:    buf[2] = '1'; break;
      case rtRight:   buf[2] = '2'; break;
      default:        buf[2] = '0'; break;
   }

   return buffer_t(buf, buf + sizeof(buf));
}

void LabelWriterDriverTwinTurbo::sendRollSelect(LabelWriterDriverTwinTurbo::roll_t value)
{
   buffer_t buf = getRollSelectCommand(value);
   sendCommand(buf);
}

};
