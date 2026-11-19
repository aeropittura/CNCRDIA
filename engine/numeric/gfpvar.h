/*
 * numeric/gfpvar.h
 *
 */

#pragma once

#include "numeric/modp.h"
#include "numeric/zp_data.h"
#include "numeric/setup.h"
#include "numeric/square.h"

class FftData;
template<class T> class BitVector_;

/**
 * Type for values in a field defined by integers modulo a prime
 * up to a certain length for fixed storage.
 * ``X`` is a counter to allow several moduli being used at the same time.
 * ``L`` is the maximum number of 64-bit limbs, that is,
 * the prime has to have bit length at most `64*L`.
 * The interface replicates ``Gfp_``.
 */
template<int X, int L>
class Gfpvar_ : public ValueInterface
{
    typedef Modp_<L> ModpType;

    static ZpData zp_d;

    ModpType a;

public:
    typedef Gfpvar_ Scalar;
    typedef FftData FD;

    typedef ::Square<Gfpvar_> Square;
    typedef Gfpvar_ next;
    typedef Gfpvar_ value_type;

    static const int MAX_N_BITS = ModpType::MAX_N_BITS;
    static const int MAX_EDABITS = ModpType::MAX_N_BITS;
    static const int N_LIMBS = ModpType::N_LIMBS;
    static const int N_BITS = -1;

    static const true_type invertible;
    static const true_type prime_field;
    static const false_type characteristic_two;

    static string type_string();
    static string type_short();
    static char type_char();

    static void specification(OctetStream& os);

    static int length();
    static int size();
    static int size_in_bits();

    static bool allows(Dtype dtype);
    static DataFieldType field_type();

    static void initialize_field(Bigint prime, bool montgomery = true);
    static void initialize_default(int lgp, bool montgomery = true);
    template<class T>
    static void init(bool montgomery)
    {
        initialize_field(T::pr(), montgomery);
    }
    static void reset();

    static const ZpData& get_zp_d();
    static const Bigint& pr();

    template<class T>
    static void generate_setup(string prep_data_prefix, int nplayers, int lgp);
    static void check_setup(string dir);
    static void write_setup(string dir);
    template<class T>
    static void write_setup(int nplayers)
    {
        write_setup(get_prep_sub_directory<T>(nplayers));
    }
    static string fake_options() { return " -P " + to_string(pr()); }

    Gfpvar_();
    Gfpvar_(int other);
    Gfpvar_(Int128 other);
    Gfpvar_(BitVector_<long> other);
    Gfpvar_(const Bigint& other);

    template<int M>
    Gfpvar_(const Modp_<M>& other, const ZpData& zp_d)
    {
        if (get_zp_d() == zp_d)
            a = other;
        else
        {
            to_bigint(Bigint::tmp, other, zp_d);
            *this = Bigint::tmp;
        }
    }

    template<int XX, int LL>
    Gfpvar_(const Gfp_<XX, LL>& other)
    {
        assert(pr() == other.pr());
        a = other.get();
    }

    template<int K>
    Gfpvar_(const Z2<K>& other) :
            Gfpvar_(Bigint(other))
    {
    }

    void assign(const void* buffer);

    void assign_zero();
    void assign_one();

    bool is_zero();
    bool is_one();
    bool is_bit();

    ModpType get() const;
    const void* get_ptr() const;
    void* get_ptr();

    void zero_overhang();
    void check();

    Gfpvar_ operator+(const Gfpvar_& other) const;
    Gfpvar_ operator-(const Gfpvar_& other) const;
    Gfpvar_ operator*(const Gfpvar_& other) const;
    Gfpvar_ operator/(const Gfpvar_& other) const;

    Gfpvar_ operator<<(int other) const;
    Gfpvar_ operator>>(int other) const;

    Gfpvar_& operator+=(const Gfpvar_& other);
    Gfpvar_& operator-=(const Gfpvar_& other);
    Gfpvar_& operator*=(const Gfpvar_& other);
    Gfpvar_& operator&=(const Gfpvar_& other);

    Gfpvar_& operator>>=(int other);

    bool operator==(const Gfpvar_& other) const;
    bool operator!=(const Gfpvar_& other) const;

    void negate();

    Gfpvar_ invert() const;

    Gfpvar_ square_root() const;

    void randomize(PRNG& G, int n_bits = -1);
    void almost_randomize(PRNG& G);

    template<int = -1>
    void pack(OctetStream& os, int n_bits = -1) const;
    void pack(OctetStream& os, int n_bits = -1) const;
    template<int = -1>
    void unpack(OctetStream& os, int n_bits = -1);
    void unpack(OctetStream& os, int n_bits = -1);

    void output(ostream& o, bool human) const;
    void input(istream& o, bool human);
};

typedef Gfpvar_<0, MAX_MOD_SZ / 2> Gfpvar;
typedef Gfpvar_<1, MAX_MOD_SZ> Gfpvar1;
typedef Gfpvar_<2, MAX_MOD_SZ> Gfpvar2;

typedef Gfpvar Gfp;

template<int X, int L>
ZpData Gfpvar_<X, L>::zp_d;

template<int X, int L>
const true_type Gfpvar_<X, L>::invertible;
template<int X, int L>
const true_type Gfpvar_<X, L>::prime_field;
template<int X, int L>
const false_type Gfpvar_<X, L>::characteristic_two;

template<int X, int L>
template<class T>
void Gfpvar_<X, L>::generate_setup(string prep_data_prefix,
    int nplayers, int lgp)
{
    generate_prime_setup<T>(prep_data_prefix, nplayers, lgp);
}

template<int X, int L>
template<int>
void Gfpvar_<X, L>::pack(OctetStream& os, int) const
{
    pack(os);
}

template<int X, int L>
template<int>
void Gfpvar_<X, L>::unpack(OctetStream& os, int)
{
    unpack(os);
}

template<int X, int L>
ostream& operator <<(ostream& o, const Gfpvar_<X, L>& x)
{
    x.output(o, true);
    return o;
}

template<int X, int L>
istream& operator >>(istream& i, Gfpvar_<X, L>& x)
{
    x.input(i, true);
    return i;
}
