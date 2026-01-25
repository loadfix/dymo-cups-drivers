#include "LabelWriterDriverTwinTurbo.h"
#include "PrinterDriver.h"

namespace DymoPrinterDriver
{

LabelWriterDriverTwinTurbo::LabelWriterDriverTwinTurbo(IPrintEnvironment& environment) :
   LabelWriterDriver400(environment), roll(ROLL_AUTO)
{
}

void LabelWriterDriverTwinTurbo::startDoc()
{
   LabelWriterDriver400::startDoc();
   sendRollSelect(roll);
}

LabelWriterDriverTwinTurbo::roll_t LabelWriterDriverTwinTurbo::getRoll()
{
   return roll;
}

void LabelWriterDriverTwinTurbo::setRoll(LabelWriterDriverTwinTurbo::roll_t value)
{
   roll = value;
}

buffer_t LabelWriterDriverTwinTurbo::getRollSelectCommand(roll_t value)
{
   byte buffer[] = {ESC, 'q', '0'};

   switch (value)
   {
      case ROLL_LEFT:    buffer[2] = '1'; break;
      case ROLL_RIGHT:   buffer[2] = '2'; break;
      default:        buffer[2] = '0'; break;
   }

   return buffer_t(buffer, buffer + sizeof(buffer));
}

void LabelWriterDriverTwinTurbo::sendRollSelect(LabelWriterDriverTwinTurbo::roll_t value)
{
   buffer_t buffer = getRollSelectCommand(value);
   sendCommand(buffer);
}

};
