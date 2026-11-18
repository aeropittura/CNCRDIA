/*
 * parties/arith/lowgear-party.cpp
 *
 */

#include "protocols/spdz/lowgear_share.h"

#include "parties/shared/spdz.hpp"
#include "protocols/spdz/cowgear_prep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
    DishonestMajorityFieldMachine<LowGearShare, LowGearShare, gf2n_short>(argc, argv, opt);
}
