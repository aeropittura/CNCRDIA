/*
 * parties/arith/cowgear-party.cpp
 *
 */

#include "protocols/spdz/cowgear_share.h"
#include "protocols/spdz/cowgear_prep.h"
#include "protocols/spdz/cowgear_options.h"

#include "he/core/fhe_params.h"
#include "he/core/fft_data.h"
#include "he/core/ntl_subs.h"

#include "boolean/tinier_secret.h"
#include "boolean/tiny_mc.h"

#include "parties/shared/spdz.hpp"

#include "protocols/spdz/cowgear_prep.hpp"
#include "vm/field_machine.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    CowGearOptions::singleton = CowGearOptions(opt, argc, argv);
    DishonestMajorityFieldMachine<CowGearShare, CowGearShare, gf2n_short>(argc, argv, opt);
}
