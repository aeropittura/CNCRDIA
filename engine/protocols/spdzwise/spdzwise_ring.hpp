/*
 * protocols/spdzwise/spdzwise_ring.hpp
 *
 */

#pragma once

#include "protocols/spdzwise/spdzwise_ring.h"

#include "protocols/replicated/post_sacrifice.hpp"

template<class T>
SpdzWiseRing<T>::SpdzWiseRing(Player& P) :
        SpdzWise<T>(P), zero_prep(0, zero_usage), zero_processor(zero_output,
                zero_prep, P)
{
}

template<class T>
void SpdzWiseRing<T>::zero_check(CheckType t)
{
    int l = T::LENGTH + T::SECURITY;
    vector<ZeroCheckType> bit_masks(l);
    ZeroCheckType masked = t;
    zero_prep.buffer_size = l;
    for (int i = 0; i < l; i++)
    {
        bit_masks[i] = zero_prep.get_bit();
        masked += bit_masks[i] << i;
    }
    auto& P = this->P;
    auto opened = zero_output.open(masked, P);
    vector<ZeroCheckType> bits(l);
    for (int i = 0; i < l; i++)
    {
        auto b = opened.get_bit(i);
        bits[i] = ZeroCheckType::constant(1 - b, P.my_num()) - bit_masks[i]
                + 2 * b * bit_masks[i];
    }
    while(bits.size() > 1)
    {
        auto& protocol = zero_processor.protocol;
        protocol.initialize_mult();
        for (int i = bits.size() - 2; i >= 0; i -= 2)
            protocol.prepare_mul(bits[i], bits[i + 1]);
        protocol.exchange();
        int n_mults = bits.size() / 2;
        bits.resize(bits.size() % 2);
        for (int i = 0; i < n_mults; i++)
            bits.push_back(protocol.finalize_mul());
    }
    zero_output.check_for(1, {bits[0]}, P);
    zero_output.Check(P);
    zero_processor.protocol.check();
}
