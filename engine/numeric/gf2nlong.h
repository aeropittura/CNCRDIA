/*
 * numeric/gf2nlong.h
 *
 */

#pragma once

#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

#include "support/random.h"
#include "support/intrinsics.h"
#include "numeric/field_types.h"
#include "numeric/bigint.h"
#include "numeric/gf2n.h"


bool is_ge(__m128i a, __m128i b);

class Int128
{
public:
    __m128i a;

    Int128() : a(_mm_setzero_si128()) { }
    Int128(const __m128i& a) : a(a) { }
    Int128(const word& a) : a(_mm_cvtsi64_si128(a)) { }
    Int128(const word& upper, const word& lower) : a(_mm_set_epi64x(upper, lower)) { }

    word get_lower() const                      { return (word)_mm_cvtsi128_si64(a); }
    word get_upper() const     { return _mm_cvtsi128_si64(_mm_unpackhi_epi64(a, a)); }
    word get_half(bool upper) const { return upper ? get_upper() : get_lower(); }

#ifdef __SSE4_1__
    bool operator==(const Int128& other) const  { return _mm_test_all_zeros(a ^ other.a, a ^ other.a); }
#else
    bool operator==(const Int128& other) const  { return get_lower() == other.get_lower() and get_upper() == other.get_upper(); }
#endif
    bool operator!=(const Int128& other) const  { return !(*this == other); }

    bool operator>=(const Int128& other) const  { return is_ge(a, other.a); }

    Int128 operator<<(const int& other) const;
    Int128 operator>>(const int& other) const;

    Int128 operator^(const Int128& other) const { return a ^ other.a; }
    Int128 operator|(const Int128& other) const { return a | other.a; }
    Int128 operator&(const Int128& other) const { return a & other.a; }

    Int128 operator~() const                    { return ~a; }

    Int128& operator<<=(const int& other)       { return *this = *this << other; }
    Int128& operator>>=(const int& other)       { return *this = *this >> other; }

    Int128& operator^=(const Int128& other)     { a ^= other.a; return *this; }
    Int128& operator|=(const Int128& other)     { a |= other.a; return *this; }
    Int128& operator&=(const Int128& other)     { a &= other.a; return *this; }

    friend ostream& operator<<(ostream& s, const Int128& a);
    friend istream& operator>>(istream& s, Int128& a);

    bool get_bit(int i) const;

    void randomize(PRNG& G)                     { *this = G.get_doubleword(); }

    void to(Int128& other)                      { other = *this; }
    void to(word& other)                        { other = get_lower(); }

    void pack(OctetStream& os) const;
    void unpack(OctetStream& os);
};

template<class T>
class BitPlus
{
    static const int N_BITS = 8 * sizeof(T);

    T lower;
    bool msb;

public:
    BitPlus() : msb(false) { }
    BitPlus(T lower, bool msb) : lower(lower), msb(msb) { }
    template<class U>
    BitPlus(U a) : lower(a), msb(false) { }
    T get_lower() { return lower; }
    bool operator==(const BitPlus& other)
        { return (lower == other.lower) && (msb == other.msb); }
    bool operator!=(const BitPlus& other)
        { return !(*this == other); }
    bool operator>=(const BitPlus& other)
        { return msb == other.msb ? lower >= other.lower : msb > other.msb; }
    BitPlus operator<<(int other)
        { return BitPlus(lower << other, ((lower >> (N_BITS-other)) & 1) != 0); }
    BitPlus& operator>>=(int other)
        { lower >>= other; lower |= (T(msb) << (N_BITS-other)); msb = !other; return *this; }
    BitPlus operator^(const BitPlus& other)
        { return BitPlus(lower ^ other.lower, msb ^ other.msb); }
    BitPlus& operator^=(const BitPlus& other)
        { lower ^= other.lower; msb ^= other.msb; return *this; }
    BitPlus operator&(const word& other)
        { return BitPlus(lower & other, false); }
    friend ostream& operator<<(ostream& s, const BitPlus& a)
        { s << a.msb << a.lower; return s; }
};

typedef BitPlus<Int128> Int129;


template<class T> class Input;
template<class T> class PrivateOutput;
template<class T> class SPDZ;
template<class T> class Share;
template<class T> class Square;

namespace GC
{
class NoValue;
}

/* This interface compatible with the gfp interface
 * which then allows us to template the Share
 * data type.
 */


/*
  Arithmetic in Gf_{2^n} with n<=128
*/

class Gf2nLong : public Gf2n_<Int128>
{
  typedef Gf2n_<Int128> Super;

  public:

  typedef Gf2nLong value_type;
  typedef Gf2nLong next;
  typedef ::Square<Gf2nLong> Square;

  typedef Gf2nLong Scalar;

  static int default_degree() { return 128; }

  static string type_string() { return "gf2n_long"; }
  word get_word() const { return this->a.get_lower(); }

  static Gf2nLong cut(Int128 x) { return x; }

  Gf2nLong()              { assign_zero(); }
  Gf2nLong(const Super& g) : Super(g) {}
  Gf2nLong(const Int128& g) : Super(g) {}
  Gf2nLong(int g) : Gf2nLong(Int128(unsigned(g))) {}
  Gf2nLong(long g) : Gf2nLong(Int128(g)) {}
  Gf2nLong(word g) : Gf2nLong(Int128(g)) {}
  template<class T>
  Gf2nLong(IntBase<T> g) : Super(g.get()) {}
  template<class T>
  Gf2nLong(const Gf2n_<T>& a) : Super(Int128(a.get())) {}
};

#if defined(__aarch64__) && defined(__clang__)
inline __m128i my_slli(Int128 x, int i)
{
  if (i < 64)
    return Int128(x.get_upper() << i, x.get_lower() << i).a;
  else
    return Int128().a;
}

inline __m128i my_srli(Int128 x, int i)
{
  if (i < 64)
    return Int128(x.get_upper() >> i, x.get_lower() >> i).a;
  else
    return Int128().a;
}

#undef _mm_slli_epi64
#undef _mm_srli_epi64
#define _mm_slli_epi64 my_slli
#define _mm_srli_epi64 my_srli
#endif

inline Int128 Int128::operator<<(const int& other) const
{
  Int128 res(_mm_slli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask = _mm_srli_epi64(a, 64 - other);
  else
    mask = _mm_slli_epi64(a, other - 64);
  res.a ^= _mm_slli_si128(mask, 8);
  return res;
}

inline Int128 Int128::operator>>(const int& other) const
{
  Int128 res(_mm_srli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask = _mm_slli_epi64(a, 64 - other);
  else
    mask = _mm_srli_epi64(a, other - 64);
  res.a ^= _mm_srli_si128(mask, 8);
  return res;
}

void mul(word x, word y, word& lo, word& hi);

inline __m128i software_clmul(__m128i a, __m128i b, int choice)
{
    word lo, hi;
    mul(Int128(a).get_half(choice & 1),
            Int128(b).get_half((choice & 0x10) >> 4), lo, hi);
    return Int128(hi, lo).a;
}

template<int choice>
inline __m128i clmul(__m128i a, __m128i b)
{
#if defined(__PCLMUL__) || !defined(__x86_64__)
    if (cpu_has_pclmul())
    {
        return _mm_clmulepi64_si128(a, b, choice);
    }
    else
#endif
        return software_clmul(a, b, choice);
}

inline void mul128(__m128i a, __m128i b, __m128i *res1, __m128i *res2)
{
    __m128i tmp3, tmp4, tmp5, tmp6;

    tmp3 = clmul<0x00>(a, b);
    tmp4 = clmul<0x10>(a, b);
    tmp5 = clmul<0x01>(a, b);
    tmp6 = clmul<0x11>(a, b);

    tmp4 = _mm_xor_si128(tmp4, tmp5);
    tmp5 = _mm_slli_si128(tmp4, 8);
    tmp4 = _mm_srli_si128(tmp4, 8);
    tmp3 = _mm_xor_si128(tmp3, tmp5);
    tmp6 = _mm_xor_si128(tmp6, tmp4);
    // initial mul now in tmp3, tmp6
    *res1 = tmp3;
    *res2 = tmp6;
}

inline void mul(Int128 a, Int128 b, Int128& lo, Int128& hi)
{
    mul128(a.a, b.a, &lo.a, &hi.a);
}

inline bool Int128::get_bit(int i) const
{
    if (i < 64)
        return (get_lower() >> i) & 1;
    else
        return (get_upper() >> (i - 64)) & 1;
}
