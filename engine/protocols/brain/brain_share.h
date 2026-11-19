/*
 * protocols/brain/brain_share.h
 *
 */

#pragma once

#include "protocols/replicated/rep3_share.h"
#include "protocols/semi/semi_share.h"

template<class T> class HashMaliciousRepOpener;
template<class T> class Beaver;
template<class T> class BrainPrep;

namespace GC
{
class MaliciousRepSecret;
}

template<int K, int S>
class BrainShare : public Rep3Share<SignedZ2<K>>
{
    typedef BrainShare This;
    typedef SignedZ2<K> T;
    typedef Rep3Share<T> Super;

public:
    typedef T clear;

    typedef Beaver<BrainShare> BasicProtocol;
    typedef HashMaliciousRepOpener<BrainShare> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ReplicatedInput<BrainShare> Input;
    typedef ::PrivateOutput<BrainShare> PrivateOutput;
    typedef BrainPrep<BrainShare> LivePrep;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    typedef GC::MaliciousRepSecret BitType;

    const static int N_MASK_BITS = clear::N_BITS + S;
    const static int Z_BITS = 2 * (N_MASK_BITS) + 5 + S;

    static const bool has_trunc_pr = false;
    static const bool malicious = true;

    BrainShare()
    {
    }
    template<class U>
    BrainShare(const FixedVec<U, 2>& other)
    {
        FixedVec<T, 2>::operator=(other);
    }
};
