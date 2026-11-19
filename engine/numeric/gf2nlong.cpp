/*
 * numeric/gf2nlong.cpp
 *
 */

#include "numeric/gf2nlong.h"
#include "numeric/gf2n.h"

#include "support/intrinsics.h"
#include "support/exceptions.h"

#include <stdint.h>


bool is_ge(__m128i a, __m128i b)
{
  word aa[2], bb[2];
  _mm_storeu_si128((__m128i*)aa, a);
  _mm_storeu_si128((__m128i*)bb, b);
//  cout << hex << "is_ge " << aa[1] << " " << bb[1] << " " << (aa[1] > bb[1]) << " ";
//  cout << aa[0] << " " << bb[0] << " " << (aa[0] >= bb[0]) << endl;
  return aa[1] == bb[1] ? aa[0] >= bb[0] : aa[1] > bb[1];
}


ostream& operator<<(ostream& s, const Int128& a)
{
  word* tmp = (word*)&a.a;
  s << hex;
  s << noshowbase;
  s.width(16);
  s.fill('0');
  s << tmp[1];
  s.width(16);
  s << tmp[0] << dec;
  return s;
}

istream& operator>>(istream& s, Int128& a)
{
  Bigint tmp;
  s >> hex >> tmp;
  a = 0;
  auto size = tmp.get_mpz_t()->_mp_size;
  assert(size >= 0);
  assert(size <= 2);
  mpn_copyi((mp_limb_t*) &a.a, tmp.get_mpz_t()->_mp_d, size);
  s >> dec;
  return s;
}

void Int128::pack(OctetStream& os) const
{
  os.store(get_lower());
  os.store(get_upper());
}

void Int128::unpack(OctetStream& os)
{
  word lower = os.get<word>();
  *this = {os.get<word>(), lower};
}
