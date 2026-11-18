/*
 * parties/arith/sy-rep-ring-party.cpp
 *
 */

#include "protocols/spdzwise/spdzwise_ring_share.h"
#include "protocols/replicated/malicious_rep3_share.h"
#include "protocols/spdzwise/spdzwise_mc.h"
#include "protocols/spdzwise/spdzwise_ring_prep.h"
#include "protocols/spdzwise/spdzwise_input.h"
#include "protocols/replicated/malrep_ring_prep.h"
#include "vm/ring_options.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/semi_honest_rep_prep.h"

#include "vm/ring_machine.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/common/share.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/spdzwise/spdzwise.hpp"
#include "protocols/spdzwise/spdzwise_ring.hpp"
#include "protocols/spdzwise/spdzwise_prep.hpp"
#include "protocols/spdzwise/spdzwise_input.hpp"
#include "protocols/spdzwise/spdzwise_share.hpp"
#include "protocols/spdzwise/spdzwise_rep3_shuffler.hpp"
#include "protocols/replicated/post_sacrifice.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/replicated/malicious_rep_prep.hpp"
#include "protocols/replicated/rep_ring_only_edabit_prep.hpp"
#include "vm/data_files.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/rep_prep.hpp"
#include "boolean/thread_master.hpp"
#include "parties/shared/malrep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HonestMajorityRingMachineWithSecurity<SpdzWiseRingShare, SpdzWiseRepFieldShare>(
            argc, argv, opt);
}
