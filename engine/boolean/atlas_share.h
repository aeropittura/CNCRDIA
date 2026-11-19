/*
 * boolean/atlas_share.h
 *
 */

#pragma once

#include "protocols/atlas/atlas_share.h"
#include "protocols/shamir/shamir_mc.h"
#include "numeric/bit.h"

namespace GC
{

class AtlasSecret;

class AtlasShare : public ::AtlasShare<Gf2n_<Octet>>, public ShareSecret<AtlasSecret>
{
    typedef AtlasShare This;

public:
    typedef ::AtlasShare<Gf2n_<Octet>> Super;

    typedef Atlas<This> Protocol;
    typedef ShamirOpener<This> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ShamirInput<This> Input;
    typedef ReplicatedPrep<This> LivePrep;

    typedef This SmallType;

    typedef Bit clear;

    static MacCheck* new_opener(MacKeyType)
    {
        return new MacCheck;
    }

    AtlasShare()
    {
    }

    template<class U>
    AtlasShare(const U& other) :
            Super(other)
    {
    }

    AtlasShare(const AtlasSecret& other);

    void XOR(const This& a, const This& b)
    {
        *this = a + b;
    }

    void public_input(bool input)
    {
        *this = input;
    }
};

}
