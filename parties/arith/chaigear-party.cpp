/*
 * parties/arith/chaigear-party.cpp
 *
 */

#include "protocols/spdz/chaigear_share.h"

#include "parties/shared/spdz.hpp"
#include "protocols/spdz/chaigear_prep.hpp"
#include "vm/field_machine.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    DishonestMajorityFieldMachine<ChaiGearShare, ChaiGearShare, Gf2nShort>(
            argc, argv, opt);
}
