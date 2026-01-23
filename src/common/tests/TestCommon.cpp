#include <ostream>
#include "TestCommon.h"

ostream&
operator<<(ostream& s, const buffer_t& b)
{
  ios::fmtflags f = s.flags(ios::hex);
  for (buffer_t::const_iterator it = b.begin(); it < b.end(); ++it)
    s << int(*it) << " ";

  s.flags(f);

  return s;
}
