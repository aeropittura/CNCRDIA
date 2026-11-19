/*
 * protocols/atlas/atlas_share.h
 *
 */

#pragma once

#include "protocols/shamir/shamir_share.h"

template<class T> class Atlas;
template<class T> class AtlasPrep;

namespace GC
{
class AtlasSecret;
}

template<class T>
class AtlasShare : public ShamirShare<T>
{
    typedef AtlasShare This;
    typedef ShamirShare<T> Super;

public:
    typedef Atlas<This> Protocol;
    typedef ShamirInput<This> Input;
    typedef IndirectShamirOpener<This> MacCheck;
    typedef ShamirOpener<This> DirectOpener;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef AtlasPrep<This> LivePrep;
    typedef LivePrep TriplePrep;

#ifndef NO_MIXED_CIRCUITS
    typedef GC::AtlasSecret BitType;
#endif

    const static int bit_generation_threshold = 2;

    static string alt()
    {
        return "";
    }

    AtlasShare()
    {
    }

    template<class U>
    AtlasShare(const U& other) :
            Super(other)
    {
    }
};
