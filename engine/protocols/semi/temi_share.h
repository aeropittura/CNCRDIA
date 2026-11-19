/*
 * protocols/semi/temi_share.h
 *
 */

#pragma once

#include "protocols/semi/hemi_share.h"

template<class T> class TemiPrep;
template<class T> class Hemi;

template<class T>
class TemiShare : public HemiShare<T>
{
    typedef TemiShare This;
    typedef HemiShare<T> Super;

public:
    typedef SemiOpener<This> MacCheck;
    typedef DirectSemiOpener<This> DirectOpener;
    typedef SemiInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef typename conditional<T::prime_field, Hemi<This>, Beaver<This>>::type Protocol;
    typedef TemiPrep<This> LivePrep;

    typedef HemiMatrixPrep<This> MatrixPrep;
    typedef Semi<This> BasicProtocol;

    static const bool needs_ot = false;
    static const bool local_mult = false;

    static string alt()
    {
        return "";
    }

    TemiShare()
    {
    }
    template<class U>
    TemiShare(const U& other) :
            Super(other)
    {
    }

};
