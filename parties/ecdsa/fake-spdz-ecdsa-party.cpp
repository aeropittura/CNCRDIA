/*
 * parties/ecdsa/fake-spdz-ecdsa-party.cpp
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
    ez::EzOptionParser opt;
    EcdsaOptions opts(opt, argc, argv);
    Names N(opt, argc, argv, 2);
    int n_tuples = 1000;
    if (not opt.last_args.empty())
        n_tuples = atoi(opt.last_args[0]->c_str());
    PlainPlayer P(N, "ecdsa");
    P256Element::init();

    P256Element::Scalar keyp;
    typedef Share<P256Element::Scalar> PShare;
    string prefix = get_prep_sub_directory<PShare>(PREP_DIR "ECDSA/", 2);
    read_mac_key(prefix, N, keyp);
    PShare::set_mac_key(keyp);

    PShare::MacCheck::setup(P);
    Share<P256Element>::MacCheck::setup(P);

    DataPositions usage;
    SubDataFiles<PShare> prep(N, prefix, usage);
    typename PShare::DirectOpener MCp(keyp);
    ArithmeticProcessor _({}, 0);
    SubProcessor<PShare> proc(_, MCp, prep, P);

    PShare sk, __;
    proc.DataF.get_two(DATA_INVERSE, sk, __);

    vector<EcTuple<Share>> tuples;
    preprocessing(tuples, n_tuples, sk, proc, opts);
    check(tuples, sk, keyp, P);
    sign_benchmark(tuples, sk, MCp, P, opts);

    PShare::MacCheck::teardown();
    Share<P256Element>::MacCheck::teardown();
    P256Element::finish();
}
