#pragma once

#include <iostream>
using namespace std;

#include <stddef.h>
#include <gmpxx.h>

#include "support/exceptions.h"
#include "support/int.h"
#include "support/random.h"
#include "support/octet_stream.h"
#include "support/avx_memcpy.h"
#include "protocols/common/config.h"

enum ReportType
{
  CAPACITY,
  USED,
  MINIMAL,
  REPORT_TYPE_MAX
};

template<int X, int L>
class Gfp_;
template<int X, int L>
class Gfpvar_;
class GmpRandom;
class Integer;
template<int K> class Z2;
template<int K> class SignedZ2;
template<int L> class Fixint;
class Int128;

namespace GC
{
  class Clear;
}

/**
 * Type for arbitrarily large integers.
 * This is a sub-class of ``mpz_class`` from GMP. As such, it implements
 * all integers operations and input/output via C++ streams. In addition,
 * the ``get_ui()`` member function allows retrieving the least significant
 * 64 bits.
 */
class Bigint : public mpz_class
{
public:
  static const false_type optimized_packing;

  static thread_local Bigint tmp, tmp2;
  static thread_local GmpRandom random;
  // workaround for GCC not always initializing thread_local variables
  static void initialize_thread() { tmp = 0; tmp2 = 0; }

  template<class T>
  static mpf_class get_float(T v, T p, T z, T s);
  template<class U, class T>
  static void output_float(U& o, const mpf_class& x, T nan);

  /// Initialize to zero.
  Bigint() : mpz_class() {}
  template <class T>
  Bigint(const T& x) : mpz_class(x) {}
  /// Convert to canonical representation as non-negative number.
  template<int X, int L>
  Bigint(const Gfp_<X, L>& x);
  /// Convert to canonical representation as non-negative number.
  template<int X, int L>
  Bigint(const Gfpvar_<X, L>& x);
  /// Convert to canonical representation as non-negative number.
  template <int K>
  Bigint(const Z2<K>& x);
  /// Convert to canonical representation as non-negative number.
  template <int K>
  Bigint(const SignedZ2<K>& x);
  template <int L>
  Bigint(const Fixint<L>& x);
  Bigint(const Integer& x);
  Bigint(const GC::Clear& x);
  Bigint(const mp_limb_t* data, size_t n_limbs);

  template<class T>
  Bigint& operator=(const T& x);
  template<int X, int L>
  Bigint& operator=(const Gfp_<X, L>& other);
  template<int X, int L>
  Bigint& operator=(const Gfpvar_<X, L>& other);
  template<int K>
  Bigint& operator=(const Z2<K>& x);
  template<int K>
  Bigint& operator=(const SignedZ2<K>& x);
  template <int L>
  Bigint& operator=(const Fixint<L>& x);
  Bigint& operator=(const Integer& x);
  Bigint& operator=(const GC::Clear& x);
  Bigint& operator=(const Int128& x);

  /// Convert to signed representation in :math:`[-p/2,p/2]`.
  template<int X, int L>
  Bigint& from_signed(const Gfp_<X, L>& other);
  template<class T>
  Bigint& from_signed(const T& other);

  void allocate_slots(const Bigint& x) { *this = x; }
  int get_min_alloc() { return get_mpz_t()->_mp_alloc; }

  void mul(const Bigint& x, const Bigint& y) { *this = x * y; }

#ifdef REALLOC_POLICE
  ~Bigint() { lottery(); }
  void lottery();

  Bigint& operator-=(const Bigint& y)
  {
    if (rand() % 10000 == 0)
      if (get_mpz_t()->_mp_alloc < abs(y.get_mpz_t()->_mp_size) + 1)
        throw runtime_error("insufficient allocation");
    ((mpz_class&)*this) -= y;
    return *this;
  }
  Bigint& operator+=(const Bigint& y)
  {
    if (rand() % 10000 == 0)
      if (get_mpz_t()->_mp_alloc < abs(y.get_mpz_t()->_mp_size) + 1)
        throw runtime_error("insufficient allocation");
    ((mpz_class&)*this) += y;
    return *this;
  }
#endif

  int num_bits() const
  { return mpz_sizeinbase(get_mpz_t(), 2); }

  void generate_uniform(PRNG& G, int n_bits, bool positive = false)
  { G.get(*this, n_bits, positive); }

  void pack(OctetStream& os, int = -1) const;
  void unpack(OctetStream& os, int = -1);

  size_t report_size(ReportType type) const;
};


void inline_mpn_zero(mp_limb_t* x, mp_size_t size);
void inline_mpn_copyi(mp_limb_t* dest, const mp_limb_t* src, mp_size_t size);


template<class T>
inline Bigint& Bigint::operator=(const T& x)
{
  mpz_class::operator=(x);
  return *this;
}

template<int K>
Bigint::Bigint(const Z2<K>& x)
{
  *this = x;
}

template<int K>
Bigint& Bigint::operator=(const Z2<K>& x)
{
  mpz_import(get_mpz_t(), Z2<K>::N_WORDS, -1,
      sizeof(typename Z2<K>::LimbType), 0, 0, x.get_ptr());
  return *this;
}

template<int K>
Bigint::Bigint(const SignedZ2<K>& x)
{
  *this = x;
}

template<int K>
Bigint& Bigint::operator=(const SignedZ2<K>& x)
{
  mpz_import(get_mpz_t(), Z2<K>::N_WORDS, -1,
      sizeof(typename SignedZ2<K>::LimbType), 0, 0, x.get_ptr());
  if (x.negative())
  {
    Bigint::tmp2 = 1;
    Bigint::tmp2 <<= K;
    *this -= Bigint::tmp2;
  }
  return *this;
}

template<int X, int L>
Bigint::Bigint(const Gfp_<X, L>& x)
{
  *this = x;
}

template<int X, int L>
Bigint::Bigint(const Gfpvar_<X, L>& other)
{
  *this = other;
}

template<int X, int L>
Bigint& Bigint::operator=(const Gfpvar_<X, L>& other)
{
  to_bigint(*this, other.get(), other.get_zp_d());
  return *this;
}

template<int X, int L>
Bigint& Bigint::operator=(const Gfp_<X, L>& x)
{
  to_bigint(*this, x);
  return *this;
}

template<class T>
void to_bigint(Bigint& res, const T& other)
{
  other.to(res);
}

template<class T>
void to_gfp(T& res, const Bigint& a)
{
  res = a;
}

template<int L>
Bigint::Bigint(const Fixint<L>& x)
{
  *this = x;
}

template<int L>
Bigint& Bigint::operator =(const Fixint<L>& x)
{
  return *this = typename Fixint<L>::Super(x);
}

string to_string(const Bigint& x);

/**********************************
 *       Utility Functions        *
 **********************************/

inline int gcd(const int x,const int y)
{
  Bigint& xx = Bigint::tmp = x;
  return mpz_gcd_ui(NULL,xx.get_mpz_t(),y);
}


inline Bigint gcd(const Bigint& x,const Bigint& y)
{ 
  Bigint g;
  mpz_gcd(g.get_mpz_t(),x.get_mpz_t(),y.get_mpz_t());
  return g;
}


inline void inverse_mod(Bigint& ans,const Bigint& x,const Bigint& p)
{
  mpz_invert(ans.get_mpz_t(),x.get_mpz_t(),p.get_mpz_t());
}

inline int num_bits(const Bigint& m)
{
  return m.num_bits();
}



inline int num_bits(long m)
{
  Bigint& te = Bigint::tmp = m;
  return mpz_sizeinbase(te.get_mpz_t(),2);
}



inline size_t numBytes(const Bigint& m)
{
  return mpz_sizeinbase(m.get_mpz_t(),256);
}





inline int prob_prime(const Bigint& x)
{
  int ans = mpz_probab_prime_p(x.get_mpz_t(), max(40, DEFAULT_SECURITY) / 2);
  return ans;
}


inline void bigint_from_bytes(Bigint& x,Octet* bytes,int len)
{
#ifdef REALLOC_POLICE
  if (rand() % 10000 == 0)
    if (x.get_mpz_t()->_mp_alloc < ((len + 7) / 8))
      throw runtime_error("insufficient allocation");
#endif
  mpz_import(x.get_mpz_t(),len,1,sizeof(Octet),0,0,bytes);
}


inline void bytes_from_bigint(Octet* bytes,const Bigint& x,unsigned int len)
{
  size_t ll = x == 0 ? 0 : numBytes(x);
  if (ll>len)
    { throw InvalidLength(); }
  memset(bytes, 0, len - ll);
  size_t l;
  mpz_export(bytes + len - ll, &l, 1, sizeof(Octet), 0, 0, x.get_mpz_t());
  assert(ll == l);
}


inline int is_odd(const Bigint& x)
{
  return mpz_odd_p(x.get_mpz_t());
}


template<class T>
Bigint square_root_mod(const T& x);

Bigint power_mod(const Bigint& x,const Bigint& e,const Bigint& p);

// Assume e>=0
int power_mod(int x,int e,int p);

inline int Hwt(int N)
{
  int result=0;
  while(N)
    { result++;
      N&=(N-1);
    }
  return result;
}

template <class T>
int limb_size();
