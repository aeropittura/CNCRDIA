/*
 * protocols/spdzwise/spdzwise_ring_share.h
 *
 */

#pragma once

#include "protocols/spdzwise/spdzwise_share.h"
#include "protocols/replicated/malicious_rep3_share.h"
#include "protocols/replicated/rep3_share2k.h"
#include "numeric/z2k.h"

template<class T> class SpdzWiseRingPrep;
template<class T> class SpdzWiseRing;

template<int K, int S>
class SpdzWiseRingShare : public SpdzWiseShare<MaliciousRep3Share<Z2<K + S>>>
{
    typedef SpdzWiseRingShare This;
    typedef SpdzWiseShare<MaliciousRep3Share<Z2<K + S>>> Super;

public:
    typedef SignedZ2<K> clear;
    typedef clear OpenType;
    typedef MaliciousRep3Share<clear> OpenPartType;

    typedef SpdzWiseOpener<This> MacCheck;
    typedef MacCheck DirectOpener;

    typedef SpdzWiseRing<This> Protocol;
    typedef SpdzWiseRingPrep<This> LivePrep;
    typedef SpdzWiseInput<This> Input;
    typedef ::PrivateOutput<This> PrivateOutput;

    typedef GC::MaliciousRepSecret BitType;

    static const int LENGTH = K;
    static const int SECURITY = S;

    static const bool has_split = true;

    SpdzWiseRingShare()
    {
    }

    template<class T>
    SpdzWiseRingShare(const T& other) :
            Super(other)
    {
    }

    template<class T, class U>
    SpdzWiseRingShare(const T &share, const U &mac) :
            Super(share, mac)
    {
    }

    template<class U>
    static void split(StackedVector<U>& dest, const ArgVector& regs, int n_bits,
            const SpdzWiseRingShare* source, int n_inputs,
            typename U::Protocol& protocol)
    {
        vector<Rep3Share2<K>> shares(n_inputs);
        for (int i = 0; i < n_inputs; i++)
            shares[i] = source[i].get_share();
        Rep3Share2<K>::split(dest, regs, n_bits, shares.data(), n_inputs, protocol);
    }

    static void shrsi(SubProcessor<This>& proc, const Instruction& inst)
    {
        typename This::part_type::Honest::Protocol protocol(proc.P);
        protocol.initialize_mult();
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_s_ref(inst.get_r(0) + i);
            auto& source = proc.get_s_ref(inst.get_r(1) + i);
            dest.set_share(Rep3Share2<K>(source.get_share()) >> inst.get_n());
            protocol.prepare_mul(dest.get_share(), proc.MC.get_alphai());
        }
        protocol.exchange();
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_s_ref(inst.get_r(0) + i);
            dest.set_mac(protocol.finalize_mul());
            proc.protocol.add_to_check(dest);
        }
    }
};
