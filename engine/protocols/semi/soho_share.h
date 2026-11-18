/*
 * protocols/semi/soho_share.h
 *
 */

#pragma once

#include "protocols/semi/semi_share.h"

template<class T> class SohoPrep;

template<class T>
class SohoShare : public SemiShare<T>
{
    typedef SohoShare This;
    typedef SemiShare<T> super;

public:
    typedef SemiMC<This> MAC_Check;
    typedef DirectSemiMC<This> Direct_MC;
    typedef SemiInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef MaybeHemi<This> Protocol;
    typedef SohoPrep<This> LivePrep;
    typedef DummyMatrixPrep<This> MatrixPrep;

    static const bool needs_ot = false;

    SohoShare()
    {
    }

    template<class U>
    SohoShare(const U& other) :
            super(other)
    {
    }
};
