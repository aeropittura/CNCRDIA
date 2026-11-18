/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#define NO_MIXED_CIRCUITS

#include "net/server.h"
#include "net/crypto_player.h"
#include "numeric/gfp.h"
#include "ecdsa/p256_element.h"
#include "boolean/vector_input.h"
#include "protocols/spdz/spdz.h"

#include "ecdsa/preprocessing.hpp"
#include "ecdsa/sign.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/semi/hemi.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/common/share.hpp"
#include "protocols/common/mac_check.hpp"
#include "vm/input.hpp"
#include "vm/processor.hpp"
#include "vm/data_files.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "boolean/secret.hpp"
#include "boolean/tiny_prep.hpp"
#include "boolean/vector_protocol.hpp"
#include "boolean/ccd_prep.hpp"
#include "ot/n_party_triple_generator.hpp"

#include <assert.h>

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    EcdsaOptions opts(opt, argc, argv);
    Names N(opt, argc, argv, 2);
    int n_tuples = 1000;
    if (not opt.lastArgs.empty())
        n_tuples = atoi(opt.lastArgs[0]->c_str());
    PlainPlayer P(N, "ecdsa");
    P256Element::init();

    P256Element::Scalar keyp;
    typedef Share<P256Element::Scalar> pShare;
    string prefix = get_prep_sub_dir<pShare>(PREP_DIR "ECDSA/", 2);
    read_mac_key(prefix, N, keyp);
    pShare::set_mac_key(keyp);

    pShare::MAC_Check::setup(P);
    Share<P256Element>::MAC_Check::setup(P);

    DataPositions usage;
    Sub_Data_Files<pShare> prep(N, prefix, usage);
    typename pShare::Direct_MC MCp(keyp);
    ArithmeticProcessor _({}, 0);
    SubProcessor<pShare> proc(_, MCp, prep, P);

    pShare sk, __;
    proc.DataF.get_two(DATA_INVERSE, sk, __);

    vector<EcTuple<Share>> tuples;
    preprocessing(tuples, n_tuples, sk, proc, opts);
    check(tuples, sk, keyp, P);
    sign_benchmark(tuples, sk, MCp, P, opts);

    pShare::MAC_Check::teardown();
    Share<P256Element>::MAC_Check::teardown();
    P256Element::finish();
}
