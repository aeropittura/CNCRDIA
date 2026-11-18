/*
 * parties/arith/emulate.cpp
 *
 */

#include "protocols/fake/fake_share.h"
#include "vm/machine.h"
#include "numeric/z2k.h"
#include "numeric/gf2n.h"
#include "vm/ring_options.h"

#include "vm/machine.hpp"
#include "vm/online_options.hpp"
#include "numeric/z2k.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/common/shuffle_sacrifice.hpp"
#include "protocols/common/replicated_prep.hpp"
#include "protocols/fake/fake_share.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/common/mac_check_base.hpp"

int main(int argc, const char** argv)
{
    OnlineOptions& online_opts = OnlineOptions::singleton;
    Names N;
    ez::ezOptionParser opt;
    RingOptions ring_opts(opt, argc, argv);
    online_opts = {opt, argc, argv, FakeShare<SignedZ2<64>>()};
    opt.syntax = string(argv[0]) + " <progname>";
    online_opts.finalize(opt, argc, argv, false);
    string& progname = online_opts.progname;

#ifdef ROUND_NEAREST_IN_EMULATION
    cerr << "Using nearest rounding instead of probabilistic truncation" << endl;
#endif

    int R = ring_opts.ring_size_from_opts_or_schedule(progname);

#define X(L) \
    if (L == R) \
    { \
        Machine<FakeShare<SignedZ2<L>>, FakeShare<gf2n>>( \
                N, false, online_opts).run(progname); \
        return 0; \
    }

    X(64)
#ifndef FEWER_RINGS
    X(32) X(128) X(256) X(192) X(384) X(512)
#endif
#ifdef RING_SIZE
    X(RING_SIZE)
#endif
#undef X
    cerr << "Not compiled for " << R << "-bit rings" << endl;
    return 1;
}
