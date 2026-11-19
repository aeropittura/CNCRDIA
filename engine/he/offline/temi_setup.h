/*
 * he/offline/temi_setup.h
 *
 */

#pragma once

#include "he/core/fhe_keys.h"
#include "he/offline/simple_machine.h"

template<class FD>
class TemiSetup : public PartSetup<FD>
{
public:
    static string name()
    {
        return "TemiParams";
    }

    static string protocol_name(int)
    {
        return "Temi";
    }

    TemiSetup();

    void secure_initialize(Player& P, MachineBase&, int plaintext_length, int, bool);
    void generate(Player& P, MachineBase&, int plaintext_length, int sec);

    void key_and_mac_generation(Player& P, MachineBase&, int, true_type);
};
