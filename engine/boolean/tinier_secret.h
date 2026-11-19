/*
 * boolean/tinier_secret.h
 *
 */

#pragma once

#include "boolean/tiny_secret.h"
#include "boolean/tinier_share.h"

template<class T> class TinierMultiplier;

namespace GC
{

template<class T> class TinierPrep;
template<class T> class VectorProtocol;
template<class T> class CcdPrep;
template<class T> class VectorInput;

template<class T>
class TinierSecret : public VectorSecret<TinierShare<T>>
{
    typedef VectorSecret<TinierShare<T>> Super;
    typedef TinierSecret This;

public:
    typedef TinyOpener<This> MC;
    typedef MC MacCheck;
    typedef VectorProtocol<This> Protocol;
    typedef VectorInput<This> Input;
    typedef CcdPrep<This> LivePrep;
    typedef Memory<This> DynamicMemory;

    typedef NPartyTripleGenerator<This> TripleGenerator;
    typedef NPartyTripleGenerator<This> InputGenerator;
    typedef TinierMultiplier<This> Multiplier;

    typedef typename Super::part_type CheckType;
    typedef Share<T> InputCheckType;
    typedef CheckType InputType;
    typedef This BitPrepType;

    static string type_short()
    {
        return "TT";
    }

    static MC* new_opener(typename Super::MacKeyType mac_key)
    {
        return new MC(mac_key);
    }

    template<class U>
    static void generate_mac_key(typename Super::MacKeyType& dest, const U&)
    {
        SeededPrng G;
        dest.randomize(G);
    }

    static void store_clear_in_dynamic(Memory<This>& mem,
            const vector<ClearWriteAccess>& accesses)
    {
        auto& party = ShareThread<This>::s();
        for (auto access : accesses)
            mem[access.address] = Super::constant(access.value,
                    party.P->my_num(), {});
    }


    TinierSecret()
    {
    }
    TinierSecret(const Super& other) :
            Super(other)
    {
    }
    TinierSecret(const typename Super::Super& other) :
            Super(other)
    {
    }
    TinierSecret(const typename Super::part_type& other)
    {
        this->get_registers().push_back(other);
    }

    void reveal(size_t n_bits, Clear& x)
    {
        auto& to_open = *this;
        to_open.resize_registers(n_bits);
        auto& party = ShareThread<This>::s();
        x = party.MC->POpen(to_open, *party.P);
    }
};

template<class T>
TinierShare<T>::TinierShare(const TinierSecret<T>& other)
{
    assert(other.get_registers().size() > 0);
    *this = other.get_reg(0);
}

} /* namespace GC */
