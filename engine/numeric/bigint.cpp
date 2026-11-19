
#include "numeric/bigint.h"
#include "numeric/gfp.h"
#include "numeric/gfpvar.h"
#include "numeric/integer.h"
#include "numeric/z2k.h"
#include "numeric/z2k.hpp"
#include "boolean/clear.h"
#include "support/exceptions.h"

#include "numeric/bigint.hpp"

thread_local Bigint Bigint::tmp = 0;
thread_local Bigint Bigint::tmp2 = 0;
thread_local GmpRandom Bigint::random;


Bigint power_mod(const Bigint& x,const Bigint& e,const Bigint& p)
{
  Bigint ans;
  if (e>=0)
    { mpz_powm(ans.get_mpz_t(),x.get_mpz_t(),e.get_mpz_t(),p.get_mpz_t()); }
  else
    { Bigint xi,ei=-e;
      inverse_mod(xi,x,p);
      mpz_powm(ans.get_mpz_t(),xi.get_mpz_t(),ei.get_mpz_t(),p.get_mpz_t()); 
    }
      
  return ans;
}


int power_mod(int x,int e,int p)
{
  if (e==1) { return x; }
  if (e==0) { return 1; }
  if (e<0)
     { throw NotImplemented(); }
   int t=x,ans=1;
   while (e!=0)
     { if ((e&1)==1) { ans=(ans*t)%p; }
       e>>=1;
       t=(t*t)%p;
     }
  return ans;
}

size_t Bigint::report_size(ReportType type) const
{
  size_t res = 0;
  if (type != MINIMAL)
    res += sizeof(*this);
  if (type == CAPACITY)
    res += get_mpz_t()->_mp_alloc * sizeof(mp_limb_t);
  else if (type == USED)
    res += abs(get_mpz_t()->_mp_size) * sizeof(mp_limb_t);
  else if (type == MINIMAL)
    res += 5 + numBytes(*this);
  return res;
}

template <>
int limb_size<Bigint>()
{
  return 64;
}

template <>
int limb_size<int>()
{
  // doesn't matter
  return 0;
}

Bigint::Bigint(const Integer& x)
{
  *this = x;
}

Bigint::Bigint(const GC::Clear& x)
{
  *this = x;
}

Bigint& Bigint::operator =(const Integer& x)
{
  return *this = SignedZ2<64>(x);
}

Bigint& Bigint::operator =(const GC::Clear& x)
{
  return *this = SignedZ2<64>(x);
}

Bigint& Bigint::operator=(const Int128& x)
{
  *this = x.get_upper();
  *this <<= 64;
  *this += x.get_lower();
  return *this;
}

Bigint::Bigint(const mp_limb_t* data, size_t n_limbs)
{
  mpz_import(get_mpz_t(), n_limbs, -1, 8, -1, 0, data);
}

void Bigint::pack(OctetStream& os, int length) const
{
  os.store(*this, length);
}

void Bigint::unpack(OctetStream& os, int length)
{
  os.get(*this, length);
}

string to_string(const Bigint& x)
{
  stringstream ss;
  ss << x;
  return ss.str();
}

#ifdef REALLOC_POLICE
void Bigint::lottery()
{
  if (rand() % 1000 == 0)
    if (rand() % 1000 == 0)
      throw runtime_error("much deallocation");
}
#endif
