/*
 * parties/arith/brain-party.cpp
 *
 */

#include "protocols/brain/brain_share.h"
#include "protocols/replicated/malicious_rep3_share.h"
#include "vm/ring_options.h"

#include "vm/ring_machine.hpp"
#include "protocols/brain/brain_prep.hpp"
#include "parties/shared/rep_ring.hpp"
#include "parties/shared/malrep.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HonestMajorityRingMachineWithSecurity<BrainShare, MaliciousRep3Share>(argc,
            argv, opt);
}
