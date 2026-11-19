/*
 * he/offline/pairwise_machine.h
 *
 */

#pragma once

#include "he/offline/pairwise_generator.h"
#include "he/offline/simple_machine.h"
#include "he/offline/pairwise_setup.h"

class PairwiseMachine : public virtual MachineBase
{
public:
    PairwiseSetup<FftData> setup_p;
    PairwiseSetup<P2Data> setup_2;
    Player& P;

    vector<FHE_PK> other_pks;
    FHE_PK& pk;
    FHE_SK sk;
    vector<Ciphertext> enc_alphas;

    PairwiseMachine(Player& P);

    template <class FD>
    PairwiseSetup<FD>& setup();

    template <class FD>
    PairwiseSetup<FD>& get_setup()
    {
        return setup<FD>();
    }

    void pack(OctetStream& os) const;
    void unpack(OctetStream& os);

    void check(Player& P) const;

    int comp_sec();
};

class RealPairwiseMachine : public virtual MachineBase, public virtual PairwiseMachine
{
public:
    RealPairwiseMachine(int argc, const char** argv);

    void init();

    template <class FD>
    void setup_keys();

    template <class T>
    void set_mac_key(T alphai);
};
