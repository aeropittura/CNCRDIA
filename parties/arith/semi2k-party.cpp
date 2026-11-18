/*
 * semi2k-party.cpp
 *
 */

#include "protocols/semi/semi2k_share.h"
#include "protocols/semi/semi_prep2k.h"
#include "numeric/gf2n.h"
#include "vm/ring_options.h"
#include "boolean/semi_prep.h"

#include "parties/shared/semi.hpp"
#include "boolean/share_secret.hpp"
#include "protocols/replicated/rep_ring_only_edabit_prep.hpp"
#include "vm/ring_machine.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityRingMachine<Semi2kShare, SemiShare>(argc, argv, opt);
}
