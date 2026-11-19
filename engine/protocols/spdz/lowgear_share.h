/*
 * protocols/spdz/lowgear_share.h
 *
 */

#pragma once

#include "protocols/spdz/cowgear_share.h"

template<class T>
class LowGearShare : public CowGearShare<T>
{
    typedef LowGearShare This;
    typedef CowGearShare<T> Super;

public:
    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef DummyMatrixPrep<This> MatrixPrep;
    typedef SPDZ<This> Protocol;
    typedef CowGearPrep<This> LivePrep;

    const static false_type covert;

    LowGearShare()
    {
    }

    template<class U>
    LowGearShare(const U& other) :
            Super(other)
    {
    }
};

template<class T>
const false_type LowGearShare<T>::covert;
