/*
 * protocols/brain/brain_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"
#include "protocols/brain/brain_share.h"

template<class T>
class BrainPrep : public MaliciousRingPrep<T>
{
    typedef gfp_<2, (T::Z_BITS + 66) / 64> gfp2;

public:
    static void basic_setup(Player&);

    BrainPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            MaliciousDabitOnlyPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage)
    {
    }

    void buffer_triples();
    void buffer_inputs(int player);
};
