/*
 * protocols/spdz/chaigear_share.h
 *
 */

#pragma once

#include "protocols/common/share.h"

template<class T> class ChaiGearPrep;

template<class T>
class ChaiGearShare : public Share<T>
{
    typedef ChaiGearShare This;
    typedef Share<T> Super;

public:
    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef DummyMatrixPrep<This> MatrixPrep;
    typedef SPDZ<This> Protocol;
    typedef ChaiGearPrep<This> LivePrep;
    typedef Share<typename T::FD::T> PrepCheckType;

    const static bool needs_ot = false;

    const static true_type covert;

    ChaiGearShare()
    {
    }

    template<class U>
    ChaiGearShare(const U& other) :
            Super(other)
    {
    }
};

template<class T>
const true_type ChaiGearShare<T>::covert;
