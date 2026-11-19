/*
 * protocols/spdzwise/spdzwise_ring.h
 *
 */

#pragma once

#include "protocols/spdzwise/spdzwise.h"
#include "protocols/replicated/post_sacrifice.h"
#include "protocols/replicated/postsacri_rep_ring_share.h"

/**
 * Three-party replicated secret sharing protocol with MAC modulo a power of two
 */
template<class T>
class SpdzWiseRing : public SpdzWise<T>
{
    typedef typename T::part_type CheckType;
    typedef PostSacriRepRingShare<T::LENGTH + T::SECURITY, T::SECURITY> ZeroCheckType;

    DataPositions zero_usage;
    MaliciousBitOnlyRepPrep<ZeroCheckType> zero_prep;
    typename ZeroCheckType::MacCheck zero_output;
    SubProcessor<ZeroCheckType> zero_processor;

public:
    SpdzWiseRing(Player &P);

    void zero_check(CheckType t);
};
