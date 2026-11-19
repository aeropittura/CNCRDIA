/*
 * protocols/replicated/postsacri_rep_field_share.h
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep3_share.h"

template<class T> class MalRepRingPrepWithBits;
template<class T> class PostSacrifice;

template<class T>
class PostSacriRepFieldShare : public MaliciousRep3Share<T>
{
    typedef MaliciousRep3Share<T> Super;
    typedef PostSacriRepFieldShare This;

public:
    typedef typename Super::clear clear;

    typedef PostSacrifice<PostSacriRepFieldShare> BasicProtocol;
    typedef HashMaliciousRepOpener<PostSacriRepFieldShare> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ReplicatedInput<PostSacriRepFieldShare> Input;
    typedef ReplicatedPo<This> PO;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef MaliciousRepPrepWithBits<PostSacriRepFieldShare> LivePrep;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    PostSacriRepFieldShare()
    {
    }
    template<class U>
    PostSacriRepFieldShare(const U& other) : Super(other)
    {
    }
};
