/*
 * protocols/spdz/cowgear_share.h
 *
 */

#pragma once

#include "protocols/common/share.h"

template<class T> class CowGearPrep;

template<class T>
class CowGearShare : public Share<T>
{
    typedef CowGearShare This;
    typedef Share<T> Super;

public:
    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef SPDZ<This> Protocol;
    typedef CowGearPrep<This> LivePrep;
    typedef DummyMatrixPrep<This> MatrixPrep;
    typedef Share<typename T::FD::T> PrepCheckType;

    const static bool needs_ot = false;

    const static true_type covert;

    CowGearShare()
    {
    }
    template<class U>
    CowGearShare(const U& other) :
            Super(other)
    {
    }

};

template<class T>
const true_type CowGearShare<T>::covert;
