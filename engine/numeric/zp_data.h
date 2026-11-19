#pragma once

/* Class to define helper information for a Zp element
 *
 * Basically the data needed for Montgomery operations 
 *
 * Almost all data is public as this is basically a container class
 *
 */

#include "numeric/config.h"
#include "numeric/bigint.h"
#include "numeric/mpn_fixed.h"
#include "support/random.h"
#include "support/intrinsics.h"
#include "support/lock.h"

#include <iostream>
using namespace std;

#ifndef MAX_MOD_SZ
   #if defined(GFP_MOD_SZ) and GFP_MOD_SZ > 11
     #define MAX_MOD_SZ 2 * GFP_MOD_SZ
   #else
     #define MAX_MOD_SZ 11
  #endif
#endif

template<int L> class Modp_;

class ZpData
{
  bool        montgomery;  // True if we are using Montgomery arithmetic
  mp_limb_t   R[MAX_MOD_SZ],R2[MAX_MOD_SZ],R3[MAX_MOD_SZ],pi;
  // extra limb needed for Montgomery multiplication
  mp_limb_t   pr_a[MAX_MOD_SZ+1];
  int         t;           // More Montgomery data
  mp_limb_t   overhang;
  Lock        lock;
  mutable Bigint shanks_y, shanks_q_half;
  mutable int    shanks_r;

  template <int T>
  void mont_mult_(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;
  void mont_mult(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;
  void mont_mult_switch(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;
  void mont_mult(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y, int t) const;
  void mont_mult_variable(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const
  { mont_mult(z, x, y, get_t()); }
  void mont_mult_max(mp_limb_t* z, const mp_limb_t* x, const mp_limb_t* y,
      int max_t) const;

  public:

  Bigint       pr;
  Bigint       pr_half;
  mp_limb_t    mask;
  size_t       pr_byte_length;
  size_t       pr_bit_length;

  void assign(const ZpData& zp);
  void init(const Bigint& p,bool mont=true);
  int get_t() const { assert(t > 0); return t; }
  const mp_limb_t* get_pr_a() const { return pr_a; }
  bool get_mont() const { return montgomery; }
  mp_limb_t overhang_mask() const;

  void pack(OctetStream& o) const;
  void unpack(OctetStream& o);

  // This one does nothing, needed so as to make vectors of ZpData
  ZpData() :
      montgomery(0), pi(0), mask(0), pr_byte_length(0), pr_bit_length(0)
  {
    t = -1;
    overhang = 0;
    shanks_r = 0;
  }

  // The main init funciton
  ZpData(const Bigint& p,bool mont=true)
    { init(p,mont); }

  template <int T>
  void Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  void Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  template <int T>
  void Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  void Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;

  bool operator!=(const ZpData& other) const;
  bool operator==(const ZpData& other) const;

  void get_shanks_parameters(Bigint& y, Bigint& q_half, int& r) const;

  void write_setup(const string& directory) const;
  void check_setup(const string& directory);

  string fake_options() const;

   template<int L> friend void to_modp(Modp_<L>& ans,int x,const ZpData& zp_d);
   template<int L> friend void to_modp(Modp_<L>& ans,const mpz_class& x,const ZpData& zp_d);

   template<int L> friend void Add(Modp_<L>& ans,const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d);
   template<int L> friend void Sub(Modp_<L>& ans,const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d);
   template<int L> friend void Mul(Modp_<L>& ans,const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d);
   template<int L> friend void Sqr(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend void Negate(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend void Inv(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d);

   template<int L> friend void Power(Modp_<L>& ans,const Modp_<L>& x,int exp,const ZpData& zp_d);
   template<int L> friend void Power(Modp_<L>& ans,const Modp_<L>& x,const Bigint& exp,const ZpData& zp_d);

   template<int L> friend void assignOne(Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend void assignZero(Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend bool isZero(const Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend bool isOne(const Modp_<L>& x,const ZpData& zp_d);
   template<int L> friend bool areEqual(const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d);

   template<int L> friend class Modp_;

   friend ostream& operator<<(ostream& s,const ZpData& zp_d);
   friend istream& operator>>(istream& s,ZpData& zp_d);
};


inline mp_limb_t ZpData::overhang_mask() const
{
  return overhang;
}

template<>
inline void ZpData::Add<0>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t carry = mpn_add_n_with_carry(ans,x,y,t);
  if (carry!=0 || mpn_cmp(ans,pr_a,t)>=0)
    { mpn_sub_n_borrow(ans,ans,pr_a,t); }
}

template<>
inline void ZpData::Add<1>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
#if defined(__clang__) || !defined(__x86_64__) || ((__GNUC__ >= 9) && (__GNUC__ <= 11)) || __GNUC__ >= 14
  Add<0>(ans, x, y);
#else
  *ans = *x + *y;
  asm goto ("jc %l[sub]" :::: sub);
  if (mpn_cmp(ans, pr_a, 1) >= 0)
 sub:
      *ans -= *pr_a;
#endif
}

template<>
inline void ZpData::Add<2>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
#if defined(__clang__) || !defined(__x86_64__) || (__GNUC__ == 9)
  Add<0>(ans, x, y);
#else
  __uint128_t a, b, p;
  memcpy(&a, x, sizeof(__uint128_t));
  memcpy(&b, y, sizeof(__uint128_t));
  memcpy(&p, pr_a, sizeof(__uint128_t));
  __uint128_t c = a + b;
  asm goto ("jc %l[sub]" :::: sub);
  if (c >= p)
 sub:
      c -= p;
  memcpy(ans, &c, sizeof(__uint128_t));
#endif
}

template<int T>
inline void ZpData::Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t carry = mpn_add_fixed_n_with_carry<T>(ans,x,y);
  if (carry!=0 || mpn_cmp(ans,pr_a,T)>=0)
    { mpn_sub_n_borrow(ans,ans,pr_a,T); }
}

inline void ZpData::Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  switch (t)
  {
#define X(L) case L: Add<L>(ans, x, y); break;
  X(1) X(2) X(3) X(4) X(5)
#undef X
  default:
    return Add<0>(ans, x, y);
  }
}

template <int T>
inline void ZpData::Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t tmp[T];
  mp_limb_t borrow = mpn_sub_fixed_n_borrow<T>(tmp, x, y);
  if (borrow != 0)
    mpn_add_fixed_n<T>(ans, tmp, pr_a);
  else
    inline_mpn_copyi(ans, tmp, T);
}

template <>
inline void ZpData::Sub<0>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t borrow = mpn_sub_n_borrow(ans,x,y,t);
  if (borrow!=0)
    mpn_add_n_with_carry(ans,ans,pr_a,t);
}

inline void ZpData::Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  switch (t)
  {
#define X(L) case L: Sub<L>(ans, x, y); break;
  X(1) X(2) X(3) X(4) X(5)
#undef X
  default:
    Sub<0>(ans, x, y);
    break;
  }
}

template <int T>
inline void ZpData::mont_mult_(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const
{
#ifdef __BMI2__
  mp_limb_t ans[2*MAX_MOD_SZ+1],u;
  inline_mpn_zero(ans + T + 1, T);
  // First loop
  u=x[0]*y[0]*pi;
  mpn_mult_1_fixed<T + 1, T>(ans,y,x[0]);
  mpn_addmul_1_fixed_<T + 2, T + 1>(ans,pr_a,u);
  for (int i=1; i<T; i++)
    { // u=(ans0+xi*y0)*pd
      u=(ans[i]+x[i]*y[0])*pi;
      // ans=ans+xi*y+u*pr
      mpn_addmul_1_fixed_<T + 2, T>(ans+i,y,x[i]);
      mpn_addmul_1_fixed_<T + 2, T + 1>(ans+i,pr_a,u);
    }
  // if (ans>=pr) { ans=z-pr; }
  // else         { z=ans;    }
  if (mpn_cmp(ans+T,pr_a,T+1)>=0)
     { mpn_sub_fixed_n<T>(z,ans+T,pr_a); }
  else
     { inline_mpn_copyi<T>(z,ans+T); }
#else
  mont_mult(z, x, y, t);
#endif
}

inline void ZpData::mont_mult(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const
{
  if (not cpu_has_bmi2())
    return mont_mult_variable(z, x, y);
#ifdef __BMI2__
  return mont_mult_switch(z, x, y);
#else
  return mont_mult_variable(z, x, y);
#endif
}

inline void ZpData::mont_mult_max(mp_limb_t* z, const mp_limb_t* x,
    const mp_limb_t* y, int max_t) const
{
  assert(t <= max_t);
  mont_mult(z, x, y);
}
