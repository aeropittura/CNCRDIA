/*
 * protocols/shamir/malicious_shamir_share.h
 *
 */

#pragma once

#include "protocols/shamir/shamir_share.h"
#include "protocols/common/beaver.h"
#include "protocols/shamir/malicious_shamir_mc.h"

template<class T> class MaliciousRepPrepWithBits;
template<class T> class MaliciousRepPrep;
template<class T> class MaliciousShamirPo;
template<class T> class SpecificPrivateOutput;

namespace GC
{
template<class T> class MaliciousCcdSecret;
}

template<class T>
class MaliciousShamirShare : public ShamirShare<T>
{
    typedef ShamirShare<T> Super;
    typedef MaliciousShamirShare This;

public:
    typedef Beaver<MaliciousShamirShare<T>> BasicProtocol;
    typedef MaliciousShamirOpener<MaliciousShamirShare> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ShamirInput<MaliciousShamirShare> Input;
    typedef MaliciousShamirPo<MaliciousShamirShare> PO;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef ShamirShare<T> Honest;
    typedef MaliciousRepPrepWithBits<MaliciousShamirShare> LivePrep;
    typedef MaliciousRepPrep<MaliciousShamirShare> TriplePrep;
    typedef T random_type;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    // indicate security relevance of field size
    typedef T MacKeyType;

#ifndef NO_MIXED_CIRCUITS
    typedef GC::MaliciousCcdSecret<Gf2nShort> BitType;
#endif

    static const bool malicious = true;

    static string type_short()
    {
        return "M" + Super::type_short();
    }

    static string alt()
    {
        return "";
    }

    MaliciousShamirShare()
    {
    }
    template<class U>
    MaliciousShamirShare(const U& other) : Super(other)
    {
    }
};
