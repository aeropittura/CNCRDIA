
#include "he/core/ring.h"
#include "support/exceptions.h"

Ring::Ring(int m) :
    mm(0), phim(0)
{
  if (m != 0)
    init(*this, m);
}

void Ring::pack(OctetStream& o) const
{
  o.store(mm);
  if (((mm - 1) & mm) != 0)
    {
      o.store(phim);
      o.store(pi);
      o.store(pi_inverse);
      o.store(poly);
    }
}

void Ring::unpack(OctetStream& o)
{
  o.get(mm);
  if (((mm - 1) & mm) != 0)
    {
      o.get(phim);
      o.get(pi);
      o.get(pi_inverse);
      o.get(poly);
    }
  else if (mm != 0)
    init(*this, mm);
}

bool Ring::operator !=(const Ring& other) const
{
  if (mm != other.mm or phim != other.phim or pi != other.pi
      or pi_inverse != other.pi_inverse or poly != other.poly)
    return true;
  else
    return false;
}
