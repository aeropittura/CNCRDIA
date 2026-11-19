/*
 * protocols/spdz/mama_share.h
 *
 */

#pragma once

#include "protocols/common/share.h"
#include "numeric/gfp.h"
#include "numeric/fixed_vec.h"
#include "ot/mama_rectangle.h"

template<class T> class MamaPrep;
template<class T> class MamaMultiplier;
template<class T> class SimpleMascotTripleGenerator;

template<class T, int N>
class MamaMac : public FixedVec<SemiShare<T>, N>
{
    typedef FixedVec<SemiShare<T>, N> Super;

public:
    typedef Super OpenType;

    static const true_type invertible;

    static int length()
    {
        return N * T::length();
    }

    MamaMac()
    {
    }

    template<class U>
    MamaMac(const U& other) :
            Super(other)
    {
    }

    MamaMac operator*(const T& other) const
    {
        return Super::operator*(SemiShare<T>(other));
    }
};

template<class T, int N>
const true_type MamaMac<T, N>::invertible;

template<class T, int N>
class MamaShare : public Share_<SemiShare<T>, MamaMac<T, N>>
{
    typedef MamaShare This;

public:
    typedef FixedVec<SemiShare<T>, N> MacKeyType;
    typedef Share_<SemiShare<T>, MamaMac<T, N>> Super;

    typedef Beaver<This> BasicProtocol;
    typedef MacCheck_<This> MacCheck;
    typedef DirectMacCheck<This> DirectOpener;
    typedef ::Input<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    typedef MamaPrep<This> LivePrep;
    typedef MamaShare<typename T::next, N> PrepType;
    typedef This BitPrepType;
    typedef This PrepCheckType;
    typedef SimpleMascotTripleGenerator<PrepType> TripleGenerator;
    typedef MascotMultiplier<This> Multiplier;
    typedef FixedVec<T, N> SacriType;
    typedef This InputType;
    typedef This InputCheckType;
    typedef MamaRectangle<T, N> Square;
    typedef typename T::Square Rectangle;
    typedef This part_type;

    static const int N_MACS = N;

    static const bool expensive = true;

    static string type_string()
    {
        return "Mama" + to_string(N) + " " + T::type_string();
    }

    static string type_short()
    {
        return "MM" + to_string(N) + string(1, T::type_char());
    }

    MamaShare()
    {
    }

    MamaShare(const Super& other) :
        Super(other)
    {
    }

    template<class U>
    MamaShare(const MamaShare<U, N>& other) :
        Super(other.get_share(), other.get_mac())
    {
    }

    template<class U, class V>
    MamaShare(const U& share, const V& mac) :
        Super(share, mac)
    {
    }
};
