/*
 * parties/offline/lowgear-offline.cpp
 *
 */

#include "parties/shared/spdz.hpp"
#include "numeric/gfp.hpp"
#include "protocols/spdz/lowgear_share.h"
#include "protocols/spdz/cowgear_options.h"
#include "protocols/spdz/cowgear_prep.hpp"
#include "vm/field_machine.hpp"
#include "vm/offline_machine.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
    DishonestMajorityFieldMachine<LowGearShare, LowGearShare, Gf2nShort,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
