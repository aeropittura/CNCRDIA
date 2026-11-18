/*
 * parties/offline/cowgear-offline.cpp
 *
 */

#include "parties/shared/spdz.hpp"
#include "numeric/gfp.hpp"
#include "protocols/spdz/cowgear_share.h"
#include "protocols/spdz/cowgear_options.h"
#include "protocols/spdz/cowgear_prep.hpp"
#include "vm/field_machine.hpp"
#include "vm/offline_machine.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    DishonestMajorityFieldMachine<CowGearShare, CowGearShare, gf2n_short,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
