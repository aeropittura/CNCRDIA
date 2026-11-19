/*
 * protocols/replicated/semi_rep3_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"

/**
 * Preprocessing for three-party replicated secret sharing modulo a power of two
 */
template<class T>
class SemiRep3Prep : public virtual SemiHonestRingPrep<T>,
        public virtual ReplicatedRingPrep<T>
{
    typedef typename T::BitType BT;

    vector<BT> b;

    void buffer_dabits(ThreadQueues*);
    template<int MY_NUM>
    void buffer_dabits_finish();

public:
    static bool bits_from_dabits() { return true; }

    SemiRep3Prep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
			RingPrep<T>(proc, usage),
            SemiHonestRingPrep<T>(proc, usage), ReplicatedRingPrep<T>(proc, usage)
    {
    }

    void buffer_bits() { this->buffer_bits_without_check(); }

    void get_one_no_count(Dtype dtype, T& a)
    {
        if (dtype != DATA_BIT)
            throw NotImplemented();

        typename T::BitType b;
        this->get_dabit_no_count(a, b);
    }
};
