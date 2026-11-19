/*
 * Gfpvar_<X, L>.cpp
 *
 */

#include "numeric/gfpvar.h"
#include "numeric/setup.h"
#include "protocols/common/share.h"

#include "numeric/gfp.hpp"

template<int X, int L>
string Gfpvar_<X, L>::type_string()
{
    return "gfp";
}

template<int X, int L>
string Gfpvar_<X, L>::type_short()
{
    return "p";
}

template<int X, int L>
char Gfpvar_<X, L>::type_char()
{
    return 'p';
}

template<int X, int L>
void Gfpvar_<X, L>::specification(OctetStream& os)
{
    zp_d.pack(os);
}

template<int X, int L>
int Gfpvar_<X, L>::length()
{
    return zp_d.pr_bit_length;
}

template<int X, int L>
int Gfpvar_<X, L>::size()
{
    return zp_d.get_t() * sizeof(mp_limb_t);
}

template<int X, int L>
int Gfpvar_<X, L>::size_in_bits()
{
    return size() * 8;
}

template<int X, int L>
bool Gfpvar_<X, L>::allows(Dtype dtype)
{
    return Gfp_<0, 0>::allows(dtype);
}

template<int X, int L>
DataFieldType Gfpvar_<X, L>::field_type()
{
    return Gfp_<0, 0>::field_type();
}

template<int X, int L>
void Gfpvar_<X, L>::initialize_field(Bigint prime, bool montgomery)
{
    zp_d.init(prime, montgomery);
    if (zp_d.get_t() > N_LIMBS)
        throw WrongGfpSize("gfpvar_<X, L>", prime, "MAX_MOD_SZ", zp_d.get_t() * 2);
}

template<int X, int L>
void Gfpvar_<X, L>::initialize_default(int lgp, bool montgomery)
{
    initialize_field(spdz_data_setup_primes(lgp), montgomery);
}

template<int X, int L>
inline void Gfpvar_<X, L>::reset()
{
    zp_d = {};
}

template<int X, int L>
const ZpData& Gfpvar_<X, L>::get_zp_d()
{
    return zp_d;
}

template<int X, int L>
const Bigint& Gfpvar_<X, L>::pr()
{
    return zp_d.pr;
}

template<int X, int L>
void Gfpvar_<X, L>::check_setup(string dir)
{
    zp_d.check_setup(dir);
}

template<int X, int L>
void Gfpvar_<X, L>::write_setup(string dir)
{
    zp_d.write_setup(dir);
}

template<int X, int L>
Gfpvar_<X, L>::Gfpvar_()
{
}

template<int X, int L>
Gfpvar_<X, L>::Gfpvar_(int other)
{
    to_modp(a, other, zp_d);
}

template<int X, int L>
Gfpvar_<X, L>::Gfpvar_(const Bigint& other)
{
    to_modp(a, other, zp_d);
}

template<int X, int L>
Gfpvar_<X, L>::Gfpvar_(Int128 other) :
        Gfpvar_(
                (Bigint::tmp = other.get_lower()
                        + ((Bigint::tmp2 = other.get_upper()) << 64)))
{
}

template<int X, int L>
Gfpvar_<X, L>::Gfpvar_(BitVector_<long> other) :
        Gfpvar_(Bigint::tmp = other.get())
{
}

template<int X, int L>
void Gfpvar_<X, L>::assign(const void* buffer)
{
    a.assign(buffer, zp_d.get_t());
}

template<int X, int L>
void Gfpvar_<X, L>::assign_zero()
{
    *this = {};
}

template<int X, int L>
void Gfpvar_<X, L>::assign_one()
{
    assignOne(a, zp_d);
}

template<int X, int L>
bool Gfpvar_<X, L>::is_zero()
{
    return isZero(a, zp_d);
}

template<int X, int L>
bool Gfpvar_<X, L>::is_one()
{
    return isOne(a, zp_d);
}

template<int X, int L>
bool Gfpvar_<X, L>::is_bit()
{
    return is_zero() or is_one();
}

template<int X, int L>
typename Gfpvar_<X, L>::ModpType Gfpvar_<X, L>::get() const
{
    return a;
}

template<int X, int L>
const void* Gfpvar_<X, L>::get_ptr() const
{
    return a.get();
}

template<int X, int L>
void* Gfpvar_<X, L>::get_ptr()
{
    return &a;
}

template<int X, int L>
void Gfpvar_<X, L>::zero_overhang()
{
    a.zero_overhang(zp_d);
}

template <int X, int L>
void Gfpvar_<X, L>::check()
{
    assert(mpn_cmp(a.get(), zp_d.get_pr_a(), zp_d.get_t()) < 0);
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator +(const Gfpvar_<X, L>& other) const
{
    Gfpvar_<X, L> res;
    Add(res.a, a, other.a, zp_d);
    return res;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator -(const Gfpvar_<X, L>& other) const
{
    Gfpvar_<X, L> res;
    Sub(res.a, a, other.a, zp_d);
    return res;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator *(const Gfpvar_<X, L>& other) const
{
    Gfpvar_<X, L> res;
    Mul(res.a, a, other.a, zp_d);
    return res;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator /(const Gfpvar_<X, L>& other) const
{
    return *this * other.invert();
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator <<(int other) const
{
    return Bigint::tmp = (Bigint::tmp = *this) << other;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::operator >>(int other) const
{
    return Bigint::tmp = (Bigint::tmp = *this) >> other;
}

template<int X, int L>
Gfpvar_<X, L>& Gfpvar_<X, L>::operator +=(const Gfpvar_<X, L>& other)
{
    Add(a, a, other.a, zp_d);
    return *this;
}

template<int X, int L>
Gfpvar_<X, L>& Gfpvar_<X, L>::operator -=(const Gfpvar_<X, L>& other)
{
    Sub(a, a, other.a, zp_d);
    return *this;
}

template<int X, int L>
Gfpvar_<X, L>& Gfpvar_<X, L>::operator *=(const Gfpvar_<X, L>& other)
{
    Mul(a, a, other.a, zp_d);
    return *this;
}

template<int X, int L>
Gfpvar_<X, L>& Gfpvar_<X, L>::operator &=(const Gfpvar_<X, L>& other)
{
    *this = Bigint::tmp = (Bigint::tmp = *this) & (Bigint::tmp2 = other);
    return *this;
}

template<int X, int L>
Gfpvar_<X, L>& Gfpvar_<X, L>::operator >>=(int other)
{
    return *this = *this >> other;
}

template<int X, int L>
bool Gfpvar_<X, L>::operator ==(const Gfpvar_<X, L>& other) const
{
    return areEqual(a, other.a, zp_d);
}

template<int X, int L>
bool Gfpvar_<X, L>::operator !=(const Gfpvar_<X, L>& other) const
{
    return not (*this == other);
}

template<int X, int L>
void Gfpvar_<X, L>::negate()
{
    *this = Gfpvar_<X, L>() - *this;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::invert() const
{
    Gfpvar_<X, L> res;
    Inv(res.a, a, zp_d);
    return res;
}

template<int X, int L>
Gfpvar_<X, L> Gfpvar_<X, L>::square_root() const
{
    Bigint ti;
    ti = square_root_mod(*this);
    if (!is_odd(ti))
        ti = zp_d.pr - ti;
    return ti;
}

template<int X, int L>
void Gfpvar_<X, L>::randomize(PRNG& G, int)
{
    a.randomize(G, zp_d);
}

template<int X, int L>
void Gfpvar_<X, L>::almost_randomize(PRNG& G)
{
    randomize(G);
}

template<int X, int L>
void Gfpvar_<X, L>::pack(OctetStream& os, int) const
{
    a.pack(os, zp_d);
}

template<int X, int L>
void Gfpvar_<X, L>::unpack(OctetStream& os, int)
{
    a.unpack(os, zp_d);
}

template<int X, int L>
void Gfpvar_<X, L>::output(ostream& o, bool human) const
{
    a.output(o, zp_d, human);
}

template<int X, int L>
void Gfpvar_<X, L>::input(istream& i, bool human)
{
    a.input(i, zp_d, human);
}

template class Gfpvar_<0, MAX_MOD_SZ / 2>;
template class Gfpvar_<1, MAX_MOD_SZ>;
template class Gfpvar_<2, MAX_MOD_SZ>;
