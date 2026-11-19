#pragma once

#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "support/random.h"

#include "numeric/field_types.h"
#include "numeric/bigint.h"
#include "numeric/value_interface.h"

class Gf2nShort;
class P2Data;
class Bit;
class Int128;
template<class T> class IntBase;
template<class T> class Square;
typedef Square<Gf2nShort> Gf2nShortSquare;

void expand_byte(Gf2nShort& a,int b);
void collapse_byte(int& b,const Gf2nShort& a);

/* This interface compatible with the gfp interface
 * which then allows us to template the Share
 * data type.
 */


/* 
  Arithmetic in Gf_{2^n} with n<64
*/

template<class U>
class Gf2n_ : public ValueInterface
{
protected:
  friend class Gf2nLong;

  U a;

  static int n, nterms;
  static int t[4], l[4];
  static U mask;
  static U uppermask, lowermask;
  static bool use_c;

  static Octet mult_table[256][256];

  static void initialize_tables();
  static void initialize_multiplication();

  template<class T>
  T invert(T a) const;

  public:

  typedef U InternalType;
  typedef Gf2n_ Scalar;

  static const int N_BYTES = sizeof(a);
  static const int MAX_N_BITS = 8 * N_BYTES;

  static void initialize_field(int nn = 0);
  static void initialize_default(int, bool = false) { initialize_field(); }
  static void initialize_minimum(int lower);

  static void reset() { n = 0; }
  static int degree() { return n; }
  static int get_nterms() { return nterms; }
  static int get_t(int i) { return (i < 3) ? t[i + 1] : -1; }

  static DataFieldType field_type() { return DATA_GF2N; }
  static char type_char() { return '2'; }
  static string type_short() { return "2"; }
  static string type_string() { return "gf2n_"; }

  static void specification(OctetStream& os);

  static int size() { return sizeof(a); }
  static int size_in_bits() { return sizeof(a) * 8; }

  static int length()         { return n == 0 ? MAX_N_BITS : n; }

  static bool allows(Dtype type) { return type <= DATA_INVERSE; }

  static string options();

  static string fake_options() { return " -lg2 " + to_string(length()); }

  static void check_setup(string) {}

  static const true_type invertible;
  static const true_type characteristic_two;
  static const true_type optimized_packing;

  static Gf2n_ Mul(Gf2n_ a, Gf2n_ b) { return a * b; }

  U get() const { return a; }

  const void* get_ptr() const { return &a; }

  void assign_zero()             { a=0; }
  void assign_one()              { a=1; } 
  void assign_x()                { a=2; }
  void assign(const void* aa)    { memcpy(&a, aa, sizeof(a)); }

  void normalize()               { a &= mask; }

  /* Assign x[0..2*nwords] to a and reduce it...  */
  void reduce(U xh, U xl);

  int get_bit(int i) const
    { return ((a>>i)&1) != 0; }
  void set_bit(int i,unsigned int b)
  { if (b==1)
      { a |= (1UL<<i); }
    else
      { a &= ~(1UL<<i); }
  }
  
  Gf2n_() : a(0)       {}
  Gf2n_(bool a) : a(a) {}
  Gf2n_(U a) : a(a & mask) {}
  Gf2n_(long a) : Gf2n_(U(a)) {}
  Gf2n_(int a) : Gf2n_(U(unsigned(a))) {}
  Gf2n_(long long a) : Gf2n_(U(a)) {}
  template<class T>
  Gf2n_(IntBase<T> a) : a(a.get()) {}

  Gf2n_(PRNG& G);

  int is_zero() const            { return (a==0); }
  int is_one()  const            { return (a==1); }
  bool operator==(const Gf2n_& y) const { return a==y.a; }
  bool operator!=(const Gf2n_& y) const { return a!=y.a; }

  // x+y
  void add(const Gf2n_& x,const Gf2n_& y)
    { a=x.a^y.a; }  
  void sub(const Gf2n_& x,const Gf2n_& y)
    { a=x.a^y.a; }
  // = x * y
  Gf2n_& mul(const Gf2n_& x,const Gf2n_& y);

  Gf2n_ lazy_add(const Gf2n_& x) const { return *this + x; }
  Gf2n_ lazy_mult(const Gf2n_& x) const { return *this * x; }

  Gf2n_ operator+(const Gf2n_& x) const { Gf2n_ res; res.add(*this, x); return res; }
  Gf2n_ operator*(const Gf2n_& x) const { Gf2n_ res; res.mul(*this, x); return res; }
  Gf2n_& operator+=(const Gf2n_& x) { add(*this, x); return *this; }
  Gf2n_& operator*=(const Gf2n_& x) { mul(*this, x); return *this; }
  Gf2n_ operator-(const Gf2n_& x) const { Gf2n_ res; res.add(*this, x); return res; }
  Gf2n_& operator-=(const Gf2n_& x) { sub(*this, x); return *this; }
  Gf2n_ operator/(const Gf2n_& x) const { return *this * x.invert(); }

  Gf2n_ operator*(const Bit& x) const;
  Gf2n_ operator*(int x) const { return *this * Gf2n_(x); }

  Gf2n_ invert() const;

  Gf2n_ operator-() const { return *this; }
  void negate() { return; }

  /* Bitwise Ops */
  Gf2n_ operator&(const Gf2n_& x) const { return a & x.a; }
  Gf2n_ operator^(const Gf2n_& x) const { return a ^ x.a; }
  Gf2n_ operator|(const Gf2n_& x) const { return a | x.a; }
  Gf2n_ operator~() const { return ~a; }
  Gf2n_ operator<<(int i) const;
  Gf2n_ operator>>(int i) const;

  Gf2n_& operator&=(const Gf2n_& x) { *this = *this & x; return *this; }
  Gf2n_& operator^=(const Gf2n_& x) { *this = *this ^ x; return *this; }
  Gf2n_& operator>>=(int i) { *this = *this >> i; return *this; }
  Gf2n_& operator<<=(int i) { *this = *this << i; return *this; }

  /* Crap RNG */
  void randomize(PRNG& G, int n = -1);
  // compatibility with gfp
  void almost_randomize(PRNG& G)        { randomize(G); }

  void force_to_bit() { a &= 1; }

  void output(ostream& s,bool human) const;
  void input(istream& s,bool human);

  friend ostream& operator<<(ostream& s,const Gf2n_& x)
    {
      x.output(s, true);
      return s;
    }
  friend istream& operator>>(istream& s,Gf2n_& x)
    {
      x.input(s, true);
      return s;
    }


  // Pack and unpack in native format
  //   i.e. Dont care about conversion to human readable form
  template<int LL = sizeof(U)>
  void pack(OctetStream& o, int n = -1) const
    { (void) n; o.append((Octet*) &a, LL); }
  template<int LL = sizeof(U)>
  void unpack(OctetStream& o, int n = -1)
    { (void) n; o.consume((Octet*) &a, LL); }
};

class Gf2nShort : public Gf2n_<word>
{
  typedef Gf2n_<word> Super;

public:
  typedef Gf2nShort value_type;
  typedef Gf2nShort next;
  typedef ::Square<Gf2nShort> Square;
  typedef P2Data FD;
  typedef Gf2nShort Scalar;

  static const int DEFAULT_LENGTH = 40;

  static int length()         { return n == 0 ? DEFAULT_LENGTH : n; }
  static int default_degree() { return DEFAULT_LENGTH; }

  static void initialize_field(int nn = 0);

  static Gf2nShort cut(Int128 x);

  Gf2nShort() {}
  template<class T>
  Gf2nShort(const T& other) : Super(other) {}
  Gf2nShort(const Int128& a);

  word get_word() const { return a; }
};

#include "numeric/gf2nlong.h"

class Gf2nLong;

#ifdef USE_GF2N_LONG
typedef Gf2nLong Gf2n;
#else
typedef Gf2nShort Gf2n;
#endif

template<class U>
const true_type Gf2n_<U>::characteristic_two;
template<class U>
const true_type Gf2n_<U>::invertible;
template<class U>
const true_type Gf2n_<U>::optimized_packing;

template<class U>
int Gf2n_<U>::n = 0;
template<class U>
U Gf2n_<U>::mask;
template<class U>
int Gf2n_<U>::nterms;
template<class U>
int Gf2n_<U>::t[4];

template<class U>
U Gf2n_<U>::uppermask;
template<class U>
U Gf2n_<U>::lowermask;

template<class U>
Octet Gf2n_<U>::mult_table[256][256];

template<>
inline Gf2n_<Octet>& Gf2n_<Octet>::mul(const Gf2n_<Octet>& x, const Gf2n_<Octet>& y)
{
  return *this = mult_table[Octet(x.a)][Octet(y.a)];
}

template<class U>
Gf2n_<U>::Gf2n_(PRNG& G)
{
  randomize(G);
}
