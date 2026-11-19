/*
 * protocols/replicated/malrep_ring_share.h
 *
 */

#pragma once

#include "numeric/z2k.h"
#include "protocols/replicated/malicious_rep3_share.h"

template<class T> class MalRepRingPrepWithBits;
template<class T> class MalRepRingPrep;

template<int K, int S>
class MalRepRingShare : public MaliciousRep3Share<SignedZ2<K>>
{
    typedef SignedZ2<K> T;
    typedef MaliciousRep3Share<T> Super;
    typedef MalRepRingShare This;

public:
    const static int BIT_LENGTH = K;
    const static int SECURITY = S;

    typedef Beaver<MalRepRingShare> BasicProtocol;
    typedef HashMaliciousRepOpener<MalRepRingShare> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ReplicatedInput<MalRepRingShare> Input;
    typedef ReplicatedPo<This> PO;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef MalRepRingPrepWithBits<MalRepRingShare> LivePrep;
    typedef MaliciousRep3Share<Z2<K + S>> PrepType;
    typedef Z2<S> random_type;
    typedef MalRepRingShare<K + 2, S> SquareToBitShare;
    typedef MalRepRingPrep<MalRepRingShare> SquarePrep;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    MalRepRingShare()
    {
    }
    template<class U>
    MalRepRingShare(const U& other) : Super(other)
    {
    }
};
