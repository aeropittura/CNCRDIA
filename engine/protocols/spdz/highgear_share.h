/*
 * protocols/spdz/highgear_share.h
 *
 */

#pragma once

#include "protocols/spdz/chaigear_share.h"

template<class T>
class HighGearShare : public ChaiGearShare<T>
{
    typedef HighGearShare This;
    typedef ChaiGearShare<T> Super;

public:
    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef DummyMatrixPrep<This> MatrixPrep;
    typedef SPDZ<This> Protocol;
    typedef ChaiGearPrep<This> LivePrep;

    const static false_type covert;

    HighGearShare()
    {
    }

    template<class U>
    HighGearShare(const U& other) :
            Super(other)
    {
    }
};

template<class T>
const false_type HighGearShare<T>::covert;
