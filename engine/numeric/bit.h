/*
 * numeric/bit.h
 *
 */

#pragma once

#include "numeric/bit_vec.h"

template<class T> class Gf2n_;

class Bit : public BitVector_<bool>
{
    typedef BitVector_<bool> Super;

public:
    static int size_in_bits()
    {
        return 1;
    }

    static int p()
    {
        return 2;
    }

    Bit()
    {
    }
    Bit(int other) :
            Super(other)
    {
        assert(other == 0 or other == 1);
    }
    Bit(const Super::Super& other) :
            Super(other)
    {
    }
    Bit(char*)
    {
        throw runtime_error("never call this");
    }

    template<class T>
    Bit(const Gf2n_<T>& other);

    Bit operator*(const Bit& other) const
    {
        return Super::operator*(other);
    }

    template<class T>
    T operator*(const T& other) const
    {
        return this->a ? other : T();
    }

    void randomize(PRNG& G)
    {
        *this = G.get_bit();
    }

    void pack(OctetStream& os, int = -1) const
    {
        Super::pack(os, 1);
    }
    void unpack(OctetStream& os, int = -1)
    {
        Super::unpack(os, 1);
    }

    void operator>>=(int)
    {
        throw runtime_error("never call this");
    }
    void operator<<=(int)
    {
        throw runtime_error("never call this");
    }
};

inline Bit operator+(bool a, const Bit& b)
{
    return b + a;
}

inline Bit operator*(bool a, const Bit& b)
{
    return b * a;
}
