/*
 * protocols/replicated/rep_ring_only_edabit_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"

/**
 * edaBit generation for replicated secret sharing modulo a power of two
 */
template<class T>
class RepRingOnlyEdabitPrep : public virtual BufferPrep<T>
{
protected:
    void buffer_edabits(int n_bits, ThreadQueues*);

public:
    static void edabit_sacrifice_buckets(vector<edabit<T>>&, size_t, bool, int,
            SubProcessor<T>&, int, int, const void* = 0)
    {
        throw runtime_error("no need for sacrifice");
    }

    RepRingOnlyEdabitPrep(SubProcessor<T>*, DataPositions& usage) :
            BufferPrep<T>(usage)
    {
    }
};
