#pragma once

#include <iostream>
using namespace std;

#include "numeric/gf2n.h"
#include "numeric/modp.h"
#include "numeric/zp_data.h"
#include "numeric/field_types.h"
#include "numeric/bit.h"
#include "numeric/setup.h"
#include "support/random.h"
#include "vm/online_options.h"

#include "numeric/modp.hpp"

/* This is a wrapper class for the modp data type
 * It is used to be interface compatible with the gfp
 * type, which then allows us to template the Share
 * data type.
 *
 * So gfp is used ONLY for the stuff in the finite fields
 * we are going to be doing MPC over, not the modp stuff
 * for the FHE scheme
 */

template<class T> class Input;
template<class T> class SPDZ;
template<class T> class Square;
class FftData;

template<class T> void generate_prime_setup(string, int, int);

#ifndef GFP_MOD_SZ
#define GFP_MOD_SZ 2
#endif

#if 2 * GFP_MOD_SZ > MAX_MOD_SZ
#error 2 * GFP_MOD_SZ must be at most MAX_MOD_SZ
#endif

/**
 * Type for values in a field defined by integers modulo a prime
 * in a specific range for fixed storage.
 * It supports basic arithmetic operations and bit-wise operations.
 * The latter use the canonical representation in the range `[0, p-1]`.
 * ``X`` is a counter to allow several moduli being used at the same time.
 * ``L`` is the number of 64-bit limbs, that is,
 * the prime has to have bit length in `[64*L-63, 64*L]`.
 * See ``Gfpvar_`` for a more flexible alternative.
 * Convert to ``bigint`` to access the canonical integer representation.
 */
template<int X, int L>
class Gfp_ : public ValueInterface
{
  typedef Modp_<L> ModpType;

  ModpType a;
  static ZpData zp_d;

  static thread_local vector<Gfp_> powers;

  static Gfp_ two;

  public:

  typedef Gfp_ value_type;
  typedef Gfp_ Scalar;

  typedef Gfp_<X + 1, L> next;
  typedef ::Square<Gfp_> Square;

  typedef FftData FD;

  static const int N_LIMBS = L;
  static const int MAX_N_BITS = 64 * L;
  static const int N_BYTES = sizeof(a);

  // must be negative
  static const int N_BITS = -1;

  static const int MAX_EDABITS = MAX_N_BITS;

  template<class T>
  static void init(bool mont = true)
    { initialize_field(T::pr(), mont); }
  /**
   * Initialize the field.
   * @param p: prime modulus
   * @param mont: whether to use Montgomery representation
   */
  static void initialize_field(const Bigint& p,bool mont=true);
  /**
   * Initialize the field to a prime of a given bit length.
   * @param lgp: bit length
   * @param mont: whether to use Montgomery representation
   */
  static void initialize_default(int lgp, bool mont = true);
  static void read_or_generate_setup(string dir, const OnlineOptions& opts);
  template<class T>
  static void generate_setup(string dir, int nplayers, int lgp)
    { generate_prime_setup<T>(dir, nplayers, lgp); }
  template<class T>
  static void write_setup(int nplayers)
    { write_setup(get_prep_sub_directory<T>(nplayers)); }
  static void write_setup(string dir)
    { zp_d.write_setup(dir); }
  static void check_setup(string dir);
  static string fake_options() { return zp_d.fake_options(); }

  /**
   * Get the prime modulus
   */
  static const Bigint& pr(bool allow_zero = false);
  static int t()
    { return L;  }
  static ZpData& get_zp_d()
    { return zp_d; }

  static DataFieldType field_type() { return DATA_INT; }
  static char type_char() { return 'p'; }
  static string type_short() { return "p"; }
  static string type_string() { return "gfp"; }

  static int size() { return t() * sizeof(mp_limb_t); }
  static int size_in_bits() { return 8 * size(); }
  static int length() { return zp_d.pr_bit_length; }
  static int n_bits() { return length() - 1; }

  static void reqbl(int n);

  static bool allows(Dtype type);

  static void specification(OctetStream& os);

  static const true_type invertible;
  static const true_type prime_field;
  static const true_type optimized_packing;

  static Gfp_ Mul(Gfp_ a, Gfp_ b) { return a * b; }

  static Gfp_ power_of_two(bool bit, int exp);

  void assign_zero()        { assignZero(a,zp_d); }
  void assign_one()         { assignOne(a,zp_d); } 
  void assign(const void* buffer) { a.assign(buffer, zp_d.get_t()); }

  ModpType get() const           { return a; }

  unsigned long debug() const { return a.get_limb(0); }

  const void* get_ptr() const { return &a.x; }
  void* get_ptr()             { return &a.x; }

  /**
   * Initialize to zero.
   */
  Gfp_()              { assignZero(a,zp_d); }
  template<int LL>
  Gfp_(const Modp_<LL>& g) { a=g; }
  /**
   * Convert from integer without range restrictions.
   */
  Gfp_(const mpz_class& x) { to_modp(a, x, zp_d); }
  Gfp_(int x) : Gfp_(long(x)) {}
  Gfp_(long x);
  Gfp_(long long x) : Gfp_(long(x)) {}
  Gfp_(word x) : Gfp_(Bigint::tmp = x) {}
  template<class T>
  Gfp_(IntBase<T> x) : Gfp_(x.get()) {}
  Gfp_(const Int128& x) : Gfp_(Bigint::tmp = x) {}
  /**
   * Convert from different domain via canonical integer representation.
   */
  template<int Y>
  Gfp_(const Gfp_<Y, L>& x);
  Gfp_(const Gfpvar& other);
  template<int K>
  Gfp_(const SignedZ2<K>& other);

  Gfp_(PRNG& G);

  void zero_overhang();
  void check();

  bool is_zero() const            { return isZero(a,zp_d); }
  bool is_one()  const            { return isOne(a,zp_d); }
  bool is_bit()  const            { return is_zero() or is_one(); }
  bool equal(const Gfp_& y) const  { return areEqual(a,y.a,zp_d); }
  bool operator==(const Gfp_& y) const { return equal(y); }
  bool operator!=(const Gfp_& y) const { return !equal(y); }

  // x+y
  void add(const Gfp_& x,const Gfp_& y)
    { zp_d.Add<L>(a.x,x.a.x,y.a.x); }
  void sub(const Gfp_& x,const Gfp_& y)
    { zp_d.Sub<L>(a.x,x.a.x,y.a.x); }
  // = x * y
  void mul(const Gfp_& x,const Gfp_& y)
    { a.template mul<L>(x.a,y.a,zp_d); }

  Gfp_ lazy_add(const Gfp_& x) const { return *this + x; }
  Gfp_ lazy_mult(const Gfp_& x) const { return *this * x; }

  Gfp_ operator+(const Gfp_& x) const { Gfp_ res; res.add(*this, x); return res; }
  Gfp_ operator-(const Gfp_& x) const { Gfp_ res; res.sub(*this, x); return res; }
  Gfp_ operator*(const Gfp_& x) const { Gfp_ res; res.mul(*this, x); return res; }
  Gfp_ operator*(int x) const { Gfp_ res; res.mul(*this, x); return res; }
  Gfp_ operator/(const Gfp_& x) const { return *this * x.invert(); }
  Gfp_& operator+=(const Gfp_& x) { add(*this, x); return *this; }
  Gfp_& operator-=(const Gfp_& x) { sub(*this, x); return *this; }
  Gfp_& operator*=(const Gfp_& x) { mul(*this, x); return *this; }

  Gfp_ operator-() { Gfp_ res = *this; res.negate(); return res; }

  Gfp_ invert() const;
  void negate() 
    { Negate(a,a,zp_d); }

  bool msb() const { throw runtime_error("msb not available"); }

  bool get_bit(int i) const;

  /**
   * Deterministic square root.
   */
  Gfp_ square_root();

  /**
   * Sample with uniform distribution.
   * @param G randomness generator
   * @param n (unused)
   */
  void randomize(PRNG& G, int n = -1)
    { (void) n; a.randomize(G,zp_d); }
  // faster randomization, see implementation for explanation
  void almost_randomize(PRNG& G);

  void from_random(const Int128& other);

  /**
   * Output.
   * @param s output stream
   * @param human human-readable or binary
   * @param signed_ signed representation (range `[-p/2,p/2]` instead of `[0,p]`)
   */
  void output(ostream& s, bool human, bool signed_ = false) const
    { a.output(s,zp_d, human, signed_); }
  void input(istream& s,bool human)
    { a.input(s,zp_d,human); }

  /**
   * Human-readable output in the range `[0, p]`.
   * @param s output stream
   * @param x value
   */
  friend ostream& operator<<(ostream& s,const Gfp_& x)
    {
      x.output(s, true, false);
      return s;
    }
  /**
   * Human-readable input without range restrictions
   * @param s input stream
   * @param x value
   */
  friend istream& operator>>(istream& s,Gfp_& x)
    { x.input(s,true);
      return s;
    }

  /* Bitwise Ops 
   *   - Converts gfp args to bigints and then converts answer back to gfp
   */
  Gfp_ operator&(const Gfp_& x) { return (Bigint::tmp = *this) &= Bigint(x); }
  Gfp_ operator^(const Gfp_& x) { return (Bigint::tmp = *this) ^= Bigint(x); }
  Gfp_ operator|(const Gfp_& x) { return (Bigint::tmp = *this) |= Bigint(x); }
  Gfp_ operator<<(int i) const;
  Gfp_ operator>>(int i) const;
  Gfp_ operator<<(const Gfp_& i) const;
  Gfp_ operator>>(const Gfp_& i) const;

  Gfp_ signed_rshift(int i) const;
  Gfp_ cheap_lshift(unsigned i) const { return *this << i; }

  Gfp_& operator&=(const Gfp_& x) { *this = *this & x; return *this; }
  Gfp_& operator<<=(int i) { *this << i; return *this; }
  Gfp_& operator>>=(int i) { *this >> i; return *this; }

  void force_to_bit() { throw runtime_error("impossible"); }

  /**
   * Append to buffer in native format.
   * @param o buffer
   * @param n (unused)
   */
  void pack(OctetStream& o, int n = -1) const
    { (void) n; a.pack(o); }
  template<int LL>
  void pack(OctetStream& o) const
    { a.template pack<LL>(o); }
  /**
   * Read from buffer in native format
   * @param o buffer
   * @param n (unused)
   */
  void unpack(OctetStream& o, int n = -1)
    { (void) n; a.unpack(o); }
  template<int LL>
  void unpack(OctetStream& o)
    { a.template unpack<LL>(o); }

  void convert_destroy(Bigint& x) { a.convert_destroy(x, zp_d); }

  void to(Bigint& res) const
  {
    res = *this;
  }

  // Convert representation to and from a bigint number
  friend void to_bigint(Bigint& ans,const Gfp_& x,bool reduce=true)
    { x.a.template to_bigint<L>(ans, x.zp_d, reduce); }
  friend void to_gfp(Gfp_& ans,const Bigint& x)
    { to_modp(ans.a,x,ans.zp_d); }
};

typedef Gfp_<0, GFP_MOD_SZ> Gfp0;
typedef Gfp_<1, GFP_MOD_SZ> Gfp1;

template<int X, int L>
ZpData Gfp_<X, L>::zp_d;
template<int X, int L>
Gfp_<X, L> Gfp_<X, L>::two;

template<int X, int L>
const true_type Gfp_<X, L>::prime_field;
template<int X, int L>
const true_type Gfp_<X, L>::optimized_packing;

template<int X, int L>
thread_local vector<Gfp_<X, L>> Gfp_<X, L>::powers;

template<int X, int L>
Gfp_<X, L>::Gfp_(long x)
{
  if (x == 0)
    assign_zero();
  else if (x == 1)
    assign_one();
  else if (x == 2)
    *this = two;
  else
    *this = Bigint::tmp = x;
}

template<int X, int L>
template<int Y>
Gfp_<X, L>::Gfp_(const Gfp_<Y, L>& x)
{
  to_bigint(Bigint::tmp, x);
  *this = Bigint::tmp;
}

template<int X, int L>
template<int K>
Gfp_<X, L>::Gfp_(const SignedZ2<K>& other)
{
  if (K >= zp_d.pr_bit_length)
    *this = Bigint::tmp = other;
  else
    a.convert(abs(other).get(), other.size_in_limbs(), zp_d, other.negative());
}

template<int X, int L>
Gfp_<X, L>::Gfp_(PRNG& G) : Gfp_()
{
  randomize(G);
}

template <int X, int L>
inline void Gfp_<X, L>::zero_overhang()
{
  a.x[t() - 1] &= zp_d.overhang_mask();
}

template<class T>
void to_signed_bigint(Bigint& ans, const T& x)
{
    ans = x;
    // get sign and abs(x)
    if (ans >= T::get_zp_d().pr_half)
        ans -= T::pr();
}
