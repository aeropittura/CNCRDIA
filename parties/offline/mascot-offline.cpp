/*
 * parties/offline/mascot-offline.cpp
 *
 */

#include "boolean/tinier_secret.h"

#include "parties/shared/spdz.hpp"
#include "numeric/gfp.hpp"
#include "vm/field_machine.hpp"
#include "vm/offline_machine.hpp"
#include "protocols/spdz/mascot_prep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityFieldMachine<Share, Share, gf2n,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
