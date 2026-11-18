/*
 * KeyGen.cpp
 *
 */

#include "he/offline/data_setup.h"
#include "vm/online_options.h"

#include "protocols/spdz/highgear_key_gen.hpp"

template<>
void PartSetup<FFT_Data>::key_and_mac_generation(Player& P,
        MachineBase& machine, int, false_type)
{
    HighGearKeyGen<0, 0>(P, params).run(*this, machine);
}

template<>
void PartSetup<P2Data>::key_and_mac_generation(Player& P, MachineBase& machine,
        int, false_type)
{
    HighGearKeyGen<0, 0>(P, params).run(*this, machine);
}
