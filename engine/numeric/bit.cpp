/*
 * numeric/bit.cpp
 *
 */

#include "numeric/bit.h"
#include "numeric/gf2n.h"

template<class T>
Bit::Bit(const Gf2n_<T>& other) :
        Super(other.get_bit(0))
{
    assert(other.is_one() or other.is_zero());
}

template Bit::Bit(const Gf2n_<Octet>& other);
template Bit::Bit(const Gf2n_<word>& other);
