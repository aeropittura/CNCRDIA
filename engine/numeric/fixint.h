/*
 * numeric/fixint.h
 *
 */

#pragma once

#include "numeric/z2k.h"

template<int L>
class Fixint : public SignedZ2<64 * (L + 1)>
{
    static const int N_OVERFLOW = 60;

public:
    typedef SignedZ2<64 * (L + 1)> Super;
    typedef typename conditional<L == 0, Super, SignedZ2<64 * L>>::type PackType;

    Fixint()
    {
    }

    template<class T>
    Fixint(const T& other) :
            Super(other)
    {
        auto check = mp_limb_signed_t(this->a[this->N_WORDS - 1]) >> N_OVERFLOW;
        assert(check == 0 or check == -1);
    }

    Fixint operator-(const Fixint& other) const
    {
        return Super::operator-(other);
    }

    Fixint operator-() const
    {
        return Super::operator-();
    }

    Fixint operator^(const Fixint& other) const
    {
        assert(L == 0);
        return Fixint(this->a[0] ^ other.a[0]);
    }

    void generate_uniform(PRNG& G, int n_bits, bool positive = true)
    {
        G.get(Bigint::tmp, n_bits, positive);
        *this = Bigint::tmp;
    }

    void random_bnd(PRNG& G, const Bigint& bound, bool positive)
    {
        G.random_bnd(Bigint::tmp, bound, positive);
        *this = Bigint::tmp;
    }

    void pack(OctetStream& os) const
    {
        PackType tmp = *this;
        tmp.pack(os);
    }

    void unpack(OctetStream& os)
    {
        PackType tmp;
        tmp.unpack(os);
        *this = tmp;
    }

    int get_min_alloc() const
    {
        return this->N_BYTES;
    }

    size_t report_size(int) const
    {
        return this->N_BYTES;
    }

    template<class T>
    void allocate_slots(const T& limit)
    {
        int n_bits = this->size_in_bits();
        if (num_bits(limit) - N_OVERFLOW > n_bits)
        {
            throw runtime_error("Fixed-length integer too small. "
                    "Maybe change N_LIMBS_RAND to at least " +
                    to_string((num_bits(limit) - N_OVERFLOW) / 64));
        }
    }
};

template<int L>
Bigint operator-(const Bigint& x, const Fixint<L>& y)
{
    return x - (Bigint::tmp = y);
}

template<int L>
Bigint& operator+=(Bigint& x, const Fixint<L>& y)
{
    x += (Bigint::tmp = y);
    return x;
}

template<int L>
Bigint operator%(const Fixint<L>& x, const Bigint& y)
{
    return (Bigint::tmp = x) % y;
}

template<int L>
Fixint<L>& operator%=(Fixint<L>& x, const Bigint& y)
{
    return x = (Bigint::tmp = x) % y;
}
