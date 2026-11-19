/*
 * numeric/bit_vec.h
 *
 */

#pragma once

#include "numeric/integer.h"
#include "numeric/field_types.h"

class BitDiagonal;

template<class T>
class BitVector_ : public IntBase<T>
{
public:
    typedef IntBase<T> Super;

    typedef BitVector_ Scalar;

    typedef BitVector_ next;
    typedef BitDiagonal Square;

    static const int n_bits = sizeof(T) * 8;

    static const false_type invertible;
    static const true_type characteristic_two;
    static const true_type binary;

    static char type_char() { return 'B'; }
    static string type_short() { return "B"; }
    static DataFieldType field_type() { return DATA_GF2; }

    static bool allows(Dtype dtype) { return dtype == DATA_TRIPLE or dtype == DATA_BIT; }

    BitVector_() {}
    BitVector_(long a) : Super(a) {}
    template<class U>
    BitVector_(const IntBase<U>& a) : Super(a.get()) {}
    template<int K>
    BitVector_(const Z2<K>& a) : Super(a.get_limb(0)) {}

    BitVector_(PRNG& G);

    BitVector_ operator+(const BitVector_& other) const { return *this ^ other; }
    BitVector_ operator-(const BitVector_& other) const { return *this ^ other; }
    BitVector_ operator*(const BitVector_& other) const { return *this & other; }

    BitVector_ operator~() const { return ~this->a; }

    BitVector_ operator/(const BitVector_& other) const { (void) other; throw NotImplemented(); }

    BitVector_& operator+=(const BitVector_& other) { *this ^= other; return *this; }
    BitVector_& operator-=(const BitVector_& other) { *this ^= other; return *this; }
    BitVector_& operator*=(const BitVector_& other) { *this &= other; return *this; }

    BitVector_ operator-() const { return *this; }

    BitVector_ extend_bit() const { return -(this->a & 1); }

    void extend_bit(BitVector_& res, int) const { res = extend_bit(); }

    void mul(const BitVector_& a, const BitVector_& b) { *this = a * b; }

    void randomize(PRNG& G, int n = -1)
    {
        if (n == -1 or is_same<T, bool>())
            Super::randomize(G);
        else
        {
            assert(n <= n_bits);
            G.get_octets((Octet*) &this->a, DIV_CEIL(n, 8));
            *this = this->mask(n);
        }
    }

    template<int LL = sizeof(T)>
    void pack(OctetStream& os) const { os.store_int<LL>(this->a); }
    template<int LL = sizeof(T)>
    void unpack(OctetStream& os) { this->a = os.get_int<LL>(); }

    void pack(OctetStream& os, int n) const
    {
        if (n == -1)
            pack(os);
        else if (n < 8)
            os.store_bits(this->a, n);
        else
            os.store_int(Super::mask(n).get(), DIV_CEIL(n, 8));
    }

    void unpack(OctetStream& os, int n)
    {
        if (n == -1)
            unpack(os);
        else if (n < 8)
            this->a = os.get_bits(n);
        else
            this->a = os.get_int(DIV_CEIL(n, 8));
    }

    static BitVector_ unpack_new(OctetStream& os, int n = n_bits)
    {
        BitVector_ res;
        res.unpack(os, n);
        return res;
    }
};

typedef BitVector_<long> BitVec;

template<class T>
const false_type BitVector_<T>::invertible;
template<class T>
const true_type BitVector_<T>::characteristic_two;
template<class T>
const true_type BitVector_<T>::binary;

template<class T>
BitVector_<T>::BitVector_(PRNG& G)
{
    randomize(G);
}
