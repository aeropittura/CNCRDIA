/*
 * boolean/astra_secret.h
 *
 */

#pragma once

#include "boolean/share_secret.h"
#include "protocols/astra_trio/astra_mc.h"
#include "protocols/astra_trio/astra_share.h"
#include "support/check_vector.h"

namespace GC
{

template<class T>
class AstraSecretBase : public ShareSecret<T>
{
public:
    static void trans(StackedVector<T>& dest, const StackedVector<T>& source,
            int n_outputs, const ArgVector& args)
    {
        vector_trans(dest, source, n_outputs, args);
    }
};

template<class U>
class AstraSecret : public U, public AstraSecretBase<AstraSecret<U>>
{
    typedef AstraSecret This;
    typedef U Super;

public:
    typedef This part_type;
    typedef This SmallType;

    typedef typename Super::template OpenerTemplate<This> MC;
    typedef MC MacCheck;

    typedef NoLivePrep<This> LivePrep;
    typedef typename Super::template ProtocolTemplate<This> Protocol;
    typedef typename Super::template InputTemplate<This> Input;

    static const bool is_real = Super::is_real;
    static const bool symmetric = Super::symmetric;
    static const int default_length = Super::clear::N_BITS;

    static bool real_shares(const Player& P)
    {
        return Super::real_shares(P);
    }

    static MC* new_opener(typename Super::MacKeyType)
    {
        return new MC;
    }

    static This constant(const typename Super::clear& value, int my_num,
            typename Super::MacKeyType, int = -1)
    {
        return Super::constant(value, my_num);
    }

    AstraSecret()
    {
    }
    template <class T>
    AstraSecret(const T& other) :
            Super(other)
    {
    }

    void load_clear(int n, const Integer& x)
    {
        this->check_length(n, x);
        *this = Super::constant(x);
    }

    void bitcom(StackedVector<This>& S, const ArgVector& regs)
    {
        plain_bitcom(*this, S, regs);
    }

    void bitdec(StackedVector<This>& S, const ArgVector& regs) const
    {
        plain_bitdec(*this, S, regs);
    }
};

template<class T>
const int AstraSecret<T>::default_length;

}
