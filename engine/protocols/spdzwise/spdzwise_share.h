/*
 * protocols/spdzwise/spdzwise_share.h
 *
 */

#pragma once

#include "protocols/common/share.h"
#include "protocols/spdzwise/spdzwise.h"
#include "vm/dummy_protocol.h"

template<class T> class NoLivePrep;
template<class T> class NotImplementedInput;
template<class T> class SpdzWiseOpener;
template<class T> class SpdzWisePrep;
template<class T> class SpdzWiseInput;

namespace GC
{
class MaliciousRepSecret;
}

template<class T>
class SpdzWiseShare : public Share_<T, T>
{
    typedef Share_<T, T> Super;

public:
    typedef T part_type;
    typedef T OpenPartType;
    typedef typename T::clear clear;
    typedef typename T::OpenType OpenType;

    typedef SpdzWiseOpener<SpdzWiseShare> MacCheck;
    typedef MacCheck DirectOpener;

    typedef SpdzWise<SpdzWiseShare> Protocol;
    typedef SpdzWisePrep<SpdzWiseShare> LivePrep;
    typedef SpdzWiseInput<SpdzWiseShare> Input;
    typedef ::PrivateOutput<SpdzWiseShare> PrivateOutput;

    typedef SpdzWiseShare Honest;
    typedef LivePrep TriplePrep;

    typedef typename T::BitType BitType;

    static const bool expensive = true;

    static string type_short()
    {
        return "SY" + T::type_short();
    }

    static string type_string()
    {
        return "SPDZ-wise " + T::type_string();
    }

    static void read_or_generate_mac_key(string directory, Player& P, T& mac_key);

    static OpenType get_rec_factor(int i, int n)
    {
        return T::get_rec_factor(i, n);
    }

    static void specification(OctetStream& os)
    {
        T::specification(os);
    }

    SpdzWiseShare()
    {
    }

    template<class U>
    SpdzWiseShare(const Share_<U, U>& other) :
            Super(other)
    {
    }

    SpdzWiseShare(const T& share, const T& mac) :
            Super(share, mac)
    {
    }

    void pack(OctetStream& os, bool full = true) const;
    void pack(OctetStream& os, OpenType factor) const;
};

template<class T> class MaliciousRep3Share;
template<class T>
using SpdzWiseRepFieldShare = SpdzWiseShare<MaliciousRep3Share<T>>;
