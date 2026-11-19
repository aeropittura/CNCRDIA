/*
 * protocols/semi/hemi_share.h
 *
 */

#pragma once

#include "protocols/semi/semi_share.h"

template<class T> class HemiPrep;
template<class T> class Hemi;
template<class T> class HemiMatrixPrep;

template<class T>
class HemiShare : public SemiShare<T>
{
    typedef HemiShare This;
    typedef SemiShare<T> Super;

public:
    typedef SemiOpener<This> MacCheck;
    typedef DirectSemiOpener<This> DirectOpener;
    typedef SemiInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    // matrix multiplication only with prime order field
    typedef typename conditional<T::prime_field, Hemi<This>, Beaver<This>>::type Protocol;
    typedef HemiPrep<This> LivePrep;

    typedef HemiMatrixPrep<This> MatrixPrep;
    typedef Semi<This> BasicProtocol;

    static const bool needs_ot = false;
    static const bool local_mult = true;
    static true_type triple_matmul;

    static string alt()
    {
        return "Temi";
    }

    HemiShare()
    {
    }
    template<class U>
    HemiShare(const U& other) :
            Super(other)
    {
    }

};

template<class T>
true_type HemiShare<T>::triple_matmul;
