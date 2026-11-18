/*
 * TripleMachine.h
 *
 */

#ifndef OT_TRIPLEMACHINE_H_
#define OT_TRIPLEMACHINE_H_

#include "numeric/gf2n.h"
#include "numeric/gfp.h"
#include "numeric/z2k.h"
#include "ot/ot_triple_setup.h"
#include "ot/mascot_params.h"
#include "support/network_options.h"

class GeneratorThread;

class TripleMachine : public OfflineMachineBase, public MascotParams
{
    Names N[2];
    int nConnections;

    gf2n mac_key2;
    gfpvar1 mac_keyp;
    Z2<128> mac_keyz;

    bigint prime;

    Player* player;

    int nloops;
    bool bonding;
    int z2k, z2s;

    NetworkOptionsWithNumber network_opts;

public:
    TripleMachine(int argc, const char** argv);

    template<class T, class U>
    GeneratorThread* new_generator(OTTripleSetup& setup, int i,
            U& mac_key);

    void run();
};

#endif /* OT_TRIPLEMACHINE_H_ */
