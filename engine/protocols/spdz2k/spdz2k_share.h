/*
 * protocols/spdz2k/spdz2k_share.h
 *
 */

#pragma once

#ifndef SPDZ2K_DEFAULT_SECURITY
#define SPDZ2K_DEFAULT_SECURITY 64
#endif

#include "numeric/z2k.h"
#include "protocols/common/share.h"
#include "protocols/common/mac_check.h"
#include "vm/dummy_protocol.h"
#include "ot/rectangle.h"

template<int K, int S> class Spdz2kMultiplier;
template<class T> class Spdz2kTripleGenerator;
template<class T> class Spdz2k;

namespace GC
{
template<int S> class TinySecret;
}

template<int K, int S>
class Spdz2kShare : public Share<Z2<K + S>>
{
    typedef Spdz2kShare This;

public:
    typedef Z2<K + S> TempType;
    typedef Share<TempType> Super;

//    typedef Integer clear;
    typedef SignedZ2<K> clear;

    typedef Z2<S> MacKeyType;
    typedef Z2<K + S> OpenType;

    typedef Spdz2kShare PrepType;
    typedef Spdz2kShare<K + 2, S> BitPrepType;
    typedef Spdz2kShare<K + S, S> PrepCheckType;
    typedef Spdz2kShare InputCheckType;
    typedef Spdz2kMultiplier<K, S> Multiplier;
    typedef Spdz2kTripleGenerator<Spdz2kShare> TripleGenerator;
    typedef Z2<K + 2 * S> SacriType;
    typedef Z2kRectangle<TAU(K, S), K + S> Rectangle;
    typedef This part_type;

    typedef MacCheckZ2k<Z2<K + S>, Z2<S>, OpenType, Spdz2kShare> MacCheck;
    typedef DirectMacCheckZ2k<Spdz2kShare> DirectOpener;
    typedef ::Input<Spdz2kShare> Input;
    typedef ::PrivateOutput<Spdz2kShare> PrivateOutput;
    typedef Beaver<This> BasicProtocol;
    typedef DummyMatrixPrep<This> MatrixPrep;
    typedef Spdz2k<Spdz2kShare> Protocol;
    typedef Spdz2kPrep<Spdz2kShare> LivePrep;

#ifndef NO_MIXED_CIRCUITS
#ifdef SPDZ2K_BIT
    typedef GC::TinySecret<S> BitType;
#endif
#endif

    const static int k = K;
    const static int s = S;

    const static bool randoms_for_opens = true;

    static string type_string() { return "SPDZ2^(" + to_string(K) + "+" + to_string(S) + ")"; }
    static string type_short() { return "Z" + to_string(K) + "," + to_string(S); }

    template<class T>
    static string protocol_fake_options() { return " -Z " + to_string(K) + " -S " + to_string(S); }

    static void set_secrets_filename(string filename)
    { PrepCheckType::secrets_filename = filename; }

    Spdz2kShare() {}
    template<class T, class V>
    Spdz2kShare(const Share_<T, V>& x) : Super(x) {}
    template<class T, class V>
    Spdz2kShare(const T& share, const V& mac) : Super(share, mac) {}
};


template<int K, int S>
Spdz2kShare<K, S> operator*(const typename Spdz2kShare<K,S>::OpenType& x, Spdz2kShare<K, S>& y)
{
    return typename Spdz2kShare<K,S>::TempType(x) * typename Spdz2kShare<K,S>::Super(y);
}
