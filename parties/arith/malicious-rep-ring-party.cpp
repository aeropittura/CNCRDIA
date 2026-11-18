/*
 * mal-rep-ring-party.cpp
 *
 */

#include "protocols/replicated/malrep_ring_share.h"
#include "protocols/replicated/malrep_ring_options.h"
#include "vm/ring_machine.hpp"
#include "vm/ring_options.h"
#include "parties/shared/rep_ring.hpp"
#include "parties/shared/malrep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    MalRepRingOptions::singleton = MalRepRingOptions(opt, argc, argv);
    HonestMajorityRingMachineWithSecurity<MalRepRingShare, MaliciousRep3Share>(argc, argv, opt);
}
