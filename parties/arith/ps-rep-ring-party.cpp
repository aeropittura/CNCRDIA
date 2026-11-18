/*
 * mal-rep-ring-party.cpp
 *
 */

#include "protocols/replicated/postsacri_rep_ring_share.h"
#include "protocols/replicated/postsacri_rep_field_share.h"
#include "vm/ring_machine.hpp"
#include "vm/ring_options.h"
#include "parties/shared/rep_ring.hpp"
#include "parties/shared/malrep.hpp"
#include "protocols/replicated/post_sacrifice.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HonestMajorityRingMachineWithSecurity<PostSacriRepRingShare,
            PostSacriRepFieldShare>(argc, argv, opt);
}
