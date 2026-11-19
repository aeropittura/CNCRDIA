/*
 * parties/boolean/ccd-party.cpp
 *
 */

#include "boolean/ccd_secret.h"
#include "boolean/tiny_mc.h"
#include "boolean/ccd_prep.h"
#include "boolean/vector_input.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/secret.hpp"
#include "boolean/ccd_prep.hpp"
#include "parties/shared/shamir.hpp"

int main(int argc, const char** argv)
{
    Gf2n_<Octet>::initialize_field(8);
    ez::EzOptionParser opt;
    ShamirOptions::singleton = {opt, argc, argv};
    assert(ShamirOptions::singleton.nparties < (1 << Gf2n_<char>::length()));
    GC::ShareParty<GC::CcdSecret<Gf2n_<Octet>>>(argc, argv, opt);
}
