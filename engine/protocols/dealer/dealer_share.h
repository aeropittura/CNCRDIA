/*
 * protocols/dealer/dealer_share.h
 *
 */

#pragma once

#include "numeric/z2k.h"
#include "protocols/semi/semi_share.h"
#include "protocols/dealer/dealer_mc.h"
#include "protocols/dealer/dealer.h"
#include "protocols/dealer/dealer_input.h"

template<class T> class DealerPrep;
template<class T> class DealerInput;
template<class T> class DealerOpener;
template<class T> class DirectDealerOpener;
template<class T> class DealerMatrixPrep;
template<class T> class Hemi;

namespace GC
{
class DealerSecret;
}

template<class T> class Dealer;

template<class T>
class DealerShare : public SemiShare<T>
{
    typedef DealerShare This;
    typedef SemiShare<T> Super;

public:
    typedef GC::DealerSecret BitType;

    typedef DealerOpener<This> MacCheck;
    typedef DirectDealerOpener<This> DirectOpener;
    typedef Hemi<This> Protocol;
    typedef DealerInput<This> Input;
    typedef DealerPrep<This> LivePrep;
    typedef ::PrivateOutput<This> PrivateOutput;

    typedef DealerMatrixPrep<This> MatrixPrep;
    typedef Dealer<This> BasicProtocol;

    static false_type dishonest_majority;
    const static bool needs_ot = false;
    const static bool symmetric = false;

    static string type_short()
    {
        return "DD" + string(1, T::type_char());
    }

    static bool real_shares(const Player& P)
    {
        return P.my_num() != P.num_players() - 1;
    }

    static This constant(const T& other, int my_num,
            const typename Super::MacKeyType& = {}, int = -1)
    {
        if (my_num == 1)
            return other;
        else
            return {};
    }

    DealerShare()
    {
    }

    template<class U>
    DealerShare(const U& other) : Super(other)
    {
    }
};

template<int K>
using DealerRingShare = DealerShare<SignedZ2<K>>;

template<class T>
false_type DealerShare<T>::dishonest_majority;
