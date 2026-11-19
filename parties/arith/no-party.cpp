/*
 * parties/arith/no-party.cpp
 *
 */

#include "protocols/common/no_share.h"

#include "vm/online_machine.hpp"
#include "vm/machine.hpp"
#include "vm/online_options.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/common/replicated_prep.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "numeric/gfp.hpp"
#include "numeric/z2k.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    OnlineOptions::singleton = {opt, argc, argv, NoShare<Gf2n>()};
    OnlineMachine machine(argc, argv, opt, OnlineOptions::singleton);
    OnlineOptions::singleton.finalize(opt, argc, argv);
    machine.start_networking();
    // use primes of length 65 to 128 for arithmetic computation
    machine.run<NoShare<Gfp_<0, 2>>, NoShare<Gf2n>>();
}
