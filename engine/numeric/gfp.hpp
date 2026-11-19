#pragma once

#include "numeric/gfp.h"
#include "numeric/gfpvar.h"
#include "numeric/setup.h"

#include "support/exceptions.h"

#include "numeric/bigint.hpp"
#include "numeric/setup.hpp"

template<int X, int L>
const true_type Gfp_<X, L>::invertible;
template<int X, int L>
const int Gfp_<X, L>::MAX_N_BITS;

template<int X, int L>
inline void Gfp_<X, L>::read_or_generate_setup(string dir,
        const OnlineOptions& opts)
{
  if (opts.prime == 0)
    read_setup<Gfp_<X, L>>(dir, opts.lgp);
  else
    initialize_field(opts.prime);
}

template<int X, int L>
void Gfp_<X, L>::check_setup(string dir)
{
  zp_d.check_setup(dir);
}

template<int X, int L>
const Bigint& Gfp_<X, L>::pr(bool allow_zero)
{
  if (not allow_zero and zp_d.pr == 0)
    throw runtime_error("gfp not initialized");

  return zp_d.pr;
}

template<int X, int L>
void Gfp_<X, L>::initialize_field(const Bigint& p, bool mont)
{
  zp_d.init(p, mont);
  char name[100];
  snprintf(name, 100, "gfp_<%d, %d>", X, L);
  if (zp_d.get_t() > L)
    {
      throw WrongGfpSize(name, p, "GFP_MOD_SZ", zp_d.get_t());
    }
  if (zp_d.get_t() < L)
    {
      if (mont)
        throw WrongGfpSize(name, p, "GFP_MOD_SZ", zp_d.get_t());
      else
        cerr << name << " larger than necessary for modulus " << p << endl;
    }
  two = Bigint::tmp = 2;
}

template <int X, int L>
void Gfp_<X, L>::initialize_default(int lgp, bool mont)
{
  initialize_field(spdz_data_setup_primes(lgp), mont);
}

template<int X, int L>
Gfp_<X, L>::Gfp_(const Gfpvar& other)
{
  assert(zp_d.pr == other.get_zp_d().pr);
  a = other.get();
}

template <int X, int L>
void Gfp_<X, L>::check()
{
  assert(mpn_cmp(a.x, zp_d.get_pr_a(), t()) < 0);
}

template <int X, int L>
void Gfp_<X, L>::almost_randomize(PRNG& G)
{
  G.get_octets((Octet*)a.x,t()*sizeof(mp_limb_t));
  a.x[t()-1]&=zp_d.mask;
}


template<int X, int L>
void Gfp_<X, L>::from_random(const Int128& other)
{
    PRNG G;
    G.set_seed_from(other);
    G.next<true>();
    randomize(G);
}


template <int X, int L>
Gfp_<X, L> Gfp_<X, L>::operator<<(int n) const
{
  if (!is_zero())
    {
      if (n != 0)
        {
          return *this * power_of_two(1, n);
        }
      else
        return *this;
    }
  else
    {
      return {};
    }
}


template <int X, int L>
Gfp_<X, L> Gfp_<X, L>::operator>>(int n) const
{
  if (!is_zero())
    {
      if (n != 0)
        {
          return (Bigint::tmp = *this) >>= n;
        }
      else
        return *this;
    }
  else
    {
      return {};
    }
}


template<int X, int L>
Gfp_<X, L> Gfp_<X, L>::signed_rshift(int i) const
{
  to_signed_bigint(Bigint::tmp, *this);
  auto& tmp = Bigint::tmp;
  if (tmp < 0)
      return tmp = -(-tmp >> i);
  else
      return Bigint::tmp >>= i;
}


template <int X, int L>
Gfp_<X, L> Gfp_<X, L>::operator<<(const Gfp_& i) const
{
    return *this << (Bigint::tmp = i).get_ui();
}


template <int X, int L>
Gfp_<X, L> Gfp_<X, L>::operator>>(const Gfp_& i) const
{
    return *this >> (Bigint::tmp = i).get_ui();
}


template<int X, int L>
Gfp_<X, L> Gfp_<X, L>::invert() const
{
    Gfp_ res;
    Inv(res.a, a, zp_d);
    return res;
}

template<int X, int L>
Gfp_<X, L> Gfp_<X, L>::square_root()
{
    // Temp move to bigint so as to call square_root_mod
    Bigint ti;
    ti = square_root_mod(*this);
    if (!is_odd(ti))
        ti = zp_d.pr - ti;
    Gfp_<X, L> temp;
    to_gfp(temp, ti);
    return temp;
}

template <int X, int L>
void Gfp_<X, L>::reqbl(int n)
{
  if ((int)n > 0 && pr() < Bigint(1) << (n-1))
    {
      cerr << "Program requires a prime of bit length " << n << ". "
          << "Run with '-lgp " << n << "' or compile with '--prime "
          << pr() << "'" << endl;
      exit(1);
    }
  else if ((int)n < 0)
    {
      cerr << "Program compiled for rings not fields, "
          "run compile.py without -R" << endl;
      exit(1);
    }
}

template<int X, int L>
bool Gfp_<X, L>::allows(Dtype type)
{
    return type <= DATA_INVERSE;
}

template<int X, int L>
void Gfp_<X, L>::specification(OctetStream& os)
{
    zp_d.pack(os);
}

template <int X, int L>
Gfp_<X, L> Gfp_<X, L>::power_of_two(bool bit, int exp)
{
    if (bit)
    {
        while (exp >= int(powers.size()))
        {
            Bigint::tmp = 1;
            Bigint::tmp <<= powers.size();
            powers.push_back(Bigint::tmp);
        }
        return powers.at(exp);
    }
    else
        return 0;
}

template<int X, int L>
bool Gfp_<X, L>::get_bit(int i) const
{
    Bigint::tmp = *this;
    assert(Bigint::tmp.num_bits());
    assert(i == 0);
    return Bigint::tmp.get_ui() & 1;
}
