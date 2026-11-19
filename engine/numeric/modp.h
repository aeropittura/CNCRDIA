#pragma once

/* 
 * Currently we only support an GMP based implementation.
 *
 * What ever is type-def'd to bigint is assumed to have
 * operator overloading for all standard operators, has
 * comparison operations and istream/ostream operators >>/<<.
 *
 * All "integer" operations will be done using operator notation
 * all "modp" operations should be done using the function calls
 * below (interchange with Montgomery arithmetic).
 *
 */

#include "support/octet_stream.h"
#include "support/random.h"

#include "numeric/bigint.h"
#include "numeric/zp_data.h"

template<int L>
class Modp_
{ 
  mp_limb_t x[L];

  public: 

  static const int MAX_N_BITS = 64 * L;
  static const int N_LIMBS = L;

  // NEXT FUNCTION IS FOR DEBUG PURPOSES ONLY
  mp_limb_t get_limb(int i) const { return x[i]; }

  // use mem* functions instead of mpn_*, so the compiler can optimize
  Modp_()
    { avx_memzero(x, sizeof(x)); }
  
  template<int M>
  Modp_(const Modp_<M>& other)
    {
      inline_mpn_copyi(x, other.get(), min(L, M));
      if (L > M)
        inline_mpn_zero(x + M, L - M);
    }

  template<int X, int M>
  Modp_(const Gfp_<X, M>& other, const ZpData& zp_d) :
      Modp_()
    {
      assert(other.get_zp_d() == zp_d);
      assert(M <= L);
      inline_mpn_copyi(x, other.get().get(), M);
    }

  template<int X, int M>
  Modp_(const Gfpvar_<X, M>& other, const ZpData& zp_d) :
      Modp_()
    {
      if (other.get_zp_d() == zp_d)
        *this = other.get();
      else
        to_modp(*this, Bigint(other), zp_d);
    }

  const mp_limb_t* get() const { return x; }

  void assign(const void* buffer, int t) { memcpy(x, buffer, t * sizeof(mp_limb_t)); }

  void convert(const mp_limb_t* source, mp_size_t size, const ZpData& zp_d,
      bool negative = false);
  void convert_destroy(Bigint& source, const ZpData& zp_d);
  void convert_destroy(int source, const ZpData& zp_d) { to_modp(*this, source, zp_d); }
  template<int M>
  void convert_destroy(const Fixint<M>& source, const ZpData& zp_d);

  void zero_overhang(const ZpData& zp_d);

  void randomize(PRNG& G, const ZpData& zp_d);

  // Pack and unpack in native format
  //   i.e. Dont care about conversion to human readable form
  //   i.e. When we do montgomery we dont care about decoding
  void pack(OctetStream& o,const ZpData& zp_d) const;
  void unpack(OctetStream& o,const ZpData& zp_d);

  template<int LL = L * sizeof(mp_limb_t)>
  void pack(OctetStream& o) const;
  template<int LL = L * sizeof(mp_limb_t)>
  void unpack(OctetStream& o);

  bool operator==(const Modp_& other) const { return 0 == mpn_cmp(x, other.x, L); }
  bool operator!=(const Modp_& other) const { return not (*this == other); }


   /**********************************
    *         Modp Operations        *
    **********************************/

  // Convert representation to and from a modp number
  void to_bigint(Bigint& ans,const ZpData& zp_d,bool reduce=true) const;
  template<int M>
  void to_bigint(Bigint& ans,const ZpData& zp_d,bool reduce=true) const;

  template<int T>
  void mul(const Modp_& x, const Modp_& y, const ZpData& zp_d);

  template<int M> friend void to_modp(Modp_<M>& ans,int x,const ZpData& zp_d);
  template<int M> friend void to_modp(Modp_<M>& ans,const mpz_class& x,const ZpData& zp_d);

  Modp_ add(const Modp_& other, const ZpData& zp_d) const;
  Modp_ sub(const Modp_& other, const ZpData& zp_d) const;
  Modp_ mul(const Modp_& other, const ZpData& zp_d) const;

  friend void Add(Modp_& ans,const Modp_& x,const Modp_& y,const ZpData& zp_d)
    { zp_d.Add(ans.x, x.x, y.x); }
  template<int M> friend void Sub(Modp_<M>& ans,const Modp_<M>& x,const Modp_<M>& y,const ZpData& zp_d);
  template<int M> friend void Mul(Modp_<M>& ans,const Modp_<M>& x,const Modp_<M>& y,const ZpData& zp_d);
  template<int M> friend void Sqr(Modp_<M>& ans,const Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend void Negate(Modp_<M>& ans,const Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend void Inv(Modp_<M>& ans,const Modp_<M>& x,const ZpData& zp_d);
   
  template<int M> friend void Power(Modp_<M>& ans,const Modp_<M>& x,int exp,const ZpData& zp_d);
  template<int M> friend void Power(Modp_<M>& ans,const Modp_<M>& x,const Bigint& exp,const ZpData& zp_d);
   
  template<int M> friend void assignOne(Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend void assignZero(Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend bool isZero(const Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend bool isOne(const Modp_<M>& x,const ZpData& zp_d);
  template<int M> friend bool areEqual(const Modp_<M>& x,const Modp_<M>& y,const ZpData& zp_d);

  // Input and output from a stream
  //  - Can do in human or machine only format (later should be faster)
  //  - If human output appends a space to help with reading
  //    and also convert back/forth from Montgomery if needed
  void output(ostream& s, const ZpData& zp_d, bool human, bool signed_ = false) const;
  void input(istream& s,const ZpData& zp_d,bool human);

  template<int X, int K>
  friend class Gfp_;
};

typedef Modp_<MAX_MOD_SZ> modp;

template<int L>
inline void Modp_<L>::pack(OctetStream& o,const ZpData& zp_d) const
{
  o.append((Octet*) x,zp_d.t*sizeof(mp_limb_t));
}

template<int L>
void assignZero(Modp_<L>& x,const ZpData& zp_d)
{
  if (sizeof(x.x) <= 3 * 16 or zp_d.pr_byte_length == 0)
    // use memset to allow the compiler to optimize
    // if x.x is at most 3*128 bits
    avx_memzero(x.x, sizeof(x.x));
  else
    inline_mpn_zero(x.x, zp_d.get_t());
}

template<int L>
inline void Sub(Modp_<L>& ans,const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d)
{
  zp_d.Sub(ans.x, x.x, y.x);
}

template<int L>
inline void Mul(Modp_<L>& ans,const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d)
{
  if (zp_d.montgomery)
    { zp_d.mont_mult_max(ans.x,x.x,y.x,L); }
  else
    { //ans.x=(x.x*y.x)%zp_d.pr;
      mp_limb_t aa[2*L],q[2*L];
      mpn_mul_n(aa,x.x,y.x,zp_d.t);
      mpn_tdiv_qr(q,ans.x,0,aa,2*zp_d.t,zp_d.pr_a,zp_d.t);
    }
}

template<int L>
template<int T>
inline void Modp_<L>::mul(const Modp_<L>& x, const Modp_<L>& y, const ZpData& zp_d)
{
  if (zp_d.montgomery)
    zp_d.mont_mult_<T>(this->x, x.x, y.x);
  else
    Mul<L>(*this, x, y, zp_d);
}

template<int L>
void to_bigint(Bigint& ans,const Modp_<L>& x,const ZpData& zp_d,bool reduce=true)
{
  x.to_bigint(ans, zp_d, reduce);
}
