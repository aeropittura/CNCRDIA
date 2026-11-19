/*
 * boolean/tinier_share.h
 *
 */

#pragma once

#include "vm/dummy_protocol.h"
#include "protocols/common/share.h"
#include "numeric/bit.h"

#ifdef LONG_BIT_MAC_KEY
class Gf2nMacKey : public Gf2nLong
{
    typedef Gf2nLong Super;
#else
class Gf2nMacKey : public Gf2nShort
{
    typedef Gf2nShort Super;
#endif

public:
    Gf2nMacKey()
    {
    }

    template<class T>
    Gf2nMacKey(const T& other) :
            Super(other)
    {
    }
};

namespace GC
{

template<class T> class TinierSecret;
template<class T> class TinierSharePrep;

template<class T>
class TinierShare: public Share_<SemiShare<Bit>, SemiShare<T>>,
        public ShareSecret<TinierSecret<T>>
{
    typedef TinierShare This;

public:
    typedef Share_<SemiShare<Bit>, SemiShare<T>> Super;

    typedef T MacKeyType;
    typedef T MacType;
    typedef T SacriType;
    typedef Share<T> InputCheckType;
    typedef This PrepType;
    typedef This PrepCheckType;
    typedef This BitPrepType;

    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef TinierSharePrep<This> LivePrep;
    typedef ::Input<This> Input;
    typedef Beaver<This> Protocol;
    typedef NPartyTripleGenerator<TinierSecret<T>> TripleGenerator;

    typedef void DynamicMemory;
    typedef SwitchableOutput OutType;

    typedef This SmallType;

    typedef TinierSecret<T> WholeType;

    static const int default_length = 1;
    static const bool expensive_triples = true;

    static string name()
    {
        return "tinier share";
    }

    static string type_string()
    {
        return "Tinier";
    }

    static string type_short()
    {
        return "TT";
    }

    static ShareThread<TinierSecret<T>>& get_party()
    {
        return ShareThread<TinierSecret<T>>::s();
    }

    static MacCheck* new_opener(MacKeyType mac_key)
    {
        return new MacCheck(mac_key);
    }

    TinierShare()
    {
    }
    TinierShare(const Super& other) :
            Super(other)
    {
    }
    TinierShare(const typename Super::ShareType& share, const typename Super::MacType& mac) :
            Super(share, mac)
    {
    }
    TinierShare(const TinierSecret<T>& other);

    void XOR(const This& a, const This& b)
    {
        *this = a + b;
    }

    This operator^(const This& other) const
    {
        return *this + other;
    }

    This& operator^=(const This& other)
    {
        *this += other;
        return *this;
    }

    void public_input(bool input)
    {
        auto& party = get_party();
        *this = Super::constant(input, party.P->my_num(),
                party.MC->get_alphai());
    }

    This lsb() const
    {
        return *this;
    }

    This get_bit(int i)
    {
        assert(i == 0);
        return lsb();
    }
};

template<class T>
const int TinierShare<T>::default_length;

} /* namespace GC */
