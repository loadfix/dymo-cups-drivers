#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "../PrinterDriver.h"
#include <ostream>

using namespace std;
using namespace DymoPrinterDriver;

ostream&
operator<<(ostream& s, const buffer_t& b);

#endif // TEST_COMMON_H
