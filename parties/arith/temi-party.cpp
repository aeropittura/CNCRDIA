/*
 * temi-party.cpp
 *
 */

#include "protocols/semi/temi_share.h"
#include "numeric/gfp.h"
#include "numeric/gf2n.h"
#include "he/core/p2_data.h"
#include "support/ez_option_parser.h"
#include "boolean/semi_secret.h"
#include "boolean/semi_prep.h"

#include "vm/field_machine.hpp"
#include "protocols/semi/temi_prep.hpp"
#include "vm/data_files.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"
#include "protocols/semi/semi_prep.hpp"
#include "protocols/semi/semi_input.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/mac_check.hpp"
#include "protocols/semi/semi_mc.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/semi/hemi.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/semi_honest_rep_prep.h"
#include "boolean/semi_secret.hpp"
#include "numeric/gfp.hpp"
#include "parties/shared/semi.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HemiOptions::singleton = {opt, argc, argv};
    DishonestMajorityFieldMachine<TemiShare, TemiShare, gf2n_short>(argc, argv,
            opt);
}
