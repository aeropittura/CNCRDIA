/*
 * parties/arith/highgear-party.cpp
 *
 */

#include "protocols/spdz/highgear_share.h"

#include "parties/shared/spdz.hpp"
#include "protocols/spdz/chaigear_prep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
    DishonestMajorityFieldMachine<HighGearShare, HighGearShare, gf2n_short>(argc, argv, opt);
}
