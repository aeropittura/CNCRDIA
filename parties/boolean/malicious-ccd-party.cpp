/*
 * parties/boolean/malicious-ccd-party.cpp
 *
 */

#include "boolean/malicious_ccd_secret.h"
#include "boolean/tiny_mc.h"
#include "boolean/ccd_prep.h"
#include "boolean/vector_input.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/secret.hpp"
#include "boolean/ccd_prep.hpp"
#include "parties/shared/shamir.hpp"
#include "parties/shared/malrep.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ShamirOptions::singleton = {opt, argc, argv};
    OnlineOptions opts(opt, argc, argv);
    gf2n_short::init_minimum(opts.security_parameter);
    GC::ShareParty<GC::MaliciousCcdSecret<gf2n_short>>(argc, argv, opt);
}
