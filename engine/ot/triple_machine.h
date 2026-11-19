/*
 * ot/triple_machine.h
 *
 */

#pragma once

#include "numeric/gf2n.h"
#include "numeric/gfp.h"
#include "numeric/z2k.h"
#include "ot/ot_triple_setup.h"
#include "ot/mascot_params.h"
#include "support/network_options.h"

class GeneratorThread;

class TripleMachine : public OfflineMachineBase, public MascotParameters
{
    Names N[2];
    int n_connections;

    Gf2n mac_key2;
    Gfpvar1 mac_keyp;
    Z2<128> mac_keyz;

    Bigint prime;

    Player* player;

    int nloops;
    bool bonding;
    int z2k, z2s;

    NetworkOptionsWithNumber network_options;

public:
    TripleMachine(int argc, const char** argv);

    template<class T, class U>
    GeneratorThread* new_generator(OtTripleSetup& setup, int i,
            U& mac_key);

    void run();
};
