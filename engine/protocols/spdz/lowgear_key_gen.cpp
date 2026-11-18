/*
 * LowGearKeyGen.cpp
 *
 */

#include "he/offline/data_setup.h"
#include "vm/online_options.h"

#include "protocols/spdz/lowgear_key_gen.hpp"

template<>
void PairwiseSetup<FFT_Data>::key_and_mac_generation(Player& P,
        PairwiseMachine& machine, int, false_type)
{
    LowGearKeyGen<0>(P, machine, params).run(*this);
}

template<>
void PairwiseSetup<P2Data>::key_and_mac_generation(Player& P,
        PairwiseMachine& machine, int, false_type)
{
    LowGearKeyGen<0>(P, machine, params).run(*this);
}
