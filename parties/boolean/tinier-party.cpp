/*
 * parties/boolean/tinier-party.cpp
 *
 */

#include "boolean/tinier_secret.h"
#include "boolean/share_party.h"
#include "boolean/tiny_mc.h"
#include "boolean/vector_input.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/machine.hpp"
#include "boolean/processor.hpp"
#include "boolean/program.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/secret.hpp"
#include "boolean/tiny_prep.hpp"
#include "boolean/tinier_share_prep.hpp"
#include "boolean/ccd_prep.hpp"

#include "vm/instruction.hpp"
#include "protocols/common/mac_check.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/common/share.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    OnlineOptions opts(opt, argc, argv);
    gf2n_mac_key::init_minimum(opts.security_parameter);
    GC::simple_binary_main<GC::TinierSecret<gf2n_mac_key>>(argc, argv, 1000);
}
