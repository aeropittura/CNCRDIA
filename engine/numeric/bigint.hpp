/*
 * numeric/bigint.hpp
 *
 */

#pragma once

#include "numeric/bigint.h"
#include "numeric/integer.h"
#include "vm/base_machine.h"

template<int X, int L>
Bigint& Bigint::from_signed(const Gfp_<X, L>& other)
{
    to_signed_bigint(*this, other);
    return *this;
}

template<class T>
Bigint& Bigint::from_signed(const T& other)
{
    *this = other;
    return *this;
}

template<class T>
mpf_class Bigint::get_float(T v, T p, T z, T s)
{
    // GMP can't handle more precision in exponent
    Integer exp = Integer(p, 31).get();
    Bigint tmp;
    tmp.from_signed(v);
    if (abs(tmp) == 1)
        BaseMachine::s().mini_warning = min(BaseMachine::s().mini_warning,
                int(exp.get()));
    mpf_class res = tmp;
    if (exp > 0)
        mpf_mul_2exp(res.get_mpf_t(), res.get_mpf_t(), exp.get());
    else
        mpf_div_2exp(res.get_mpf_t(), res.get_mpf_t(), -exp.get());
    if (z.is_one())
        res = 0;
    if (s.is_one())
    {
        res *= -1;
    }
    if (not z.is_bit() or not s.is_bit())
      {
        cerr << "z=" << z << " s=" << s << endl;
        throw ProcessorError("invalid floating point number");
      }
    return res;
}

template<class U, class T>
void Bigint::output_float(U& o, const mpf_class& x, T nan)
{
    if (nan.is_zero())
        o << x;
    else
    {
        o << "NaN";
        BaseMachine::s().nan_warning = true;
    }
}


class GmpRandom
{
public:
    gmp_randclass Gen;
    GmpRandom() : Gen(gmp_randinit_default)
    {
        Gen.seed(0);
    }
};

template<class T>
Bigint square_root_mod(const T& aa)
{
    Bigint a = aa;
    Bigint p = T::pr();

    Bigint ans;
    if (a == 0)
    {
        ans = 0;
        return ans;
    }
    if (mpz_legendre(a.get_mpz_t(), p.get_mpz_t()) != 1)
        throw runtime_error("cannot compute square root of non-square");
    if (mpz_tstbit(p.get_mpz_t(), 1) == 1)
    {
        // First do case with p=3 mod 4
        Bigint exp = (p + 1) / 4;
        mpz_powm(ans.get_mpz_t(), a.get_mpz_t(), exp.get_mpz_t(),
                p.get_mpz_t());
    }
    else
    {
        // Shanks algorithm
        Bigint n, q, yy, xx, temp;
        int r;
        T::get_zp_d().get_shanks_parameters(yy, temp, r);
        mpz_powm(xx.get_mpz_t(), a.get_mpz_t(), temp.get_mpz_t(), p.get_mpz_t());
        // b=a*x^2 mod p, x=a*x mod p
        T x = xx;
        T b = (aa * x * x);
        x = (aa * x);
        T y = yy;
        // While b!=1 do
        while (b != 1)
        {
            // Find smallest m such that b^(2^m)=1 mod p
            int m = 1;
            T temp = (b * b);
            while (temp != 1)
            {
                temp = (temp * temp);
                m++;
            }
            // t=y^(2^(r-m-1)) mod p, y=t^2, r=m
            T t = y;
            for (int i = 0; i < r - m - 1; i++)
            {
                t = (t * t);
            }
            y = (t * t);
            r = m;
            // x=x*t mod p, b=b*y mod p
            x = (x * t);
            b = (b * y);
        }
        ans = x;
    }
    return ans;
}
