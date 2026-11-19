/*
 * ecdsa/hm_ecdsa_party.hpp
 *
 */

#pragma once

#include "net/server.h"
#include "net/crypto_player.h"
#include "protocols/replicated/replicated.h"
#include "protocols/replicated/malicious_rep3_share.h"
#include "protocols/replicated/replicated_input.h"
#include "protocols/atlas/atlas_share.h"
#include "protocols/rep4/rep4_share.h"
#include "protocols/common/protocol_set.h"
#include "numeric/gfp.h"
#include "ecdsa/p256_element.h"
#include "support/bundle.h"
#include "boolean/tiny_mc.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/ccd_secret.h"
#include "boolean/vector_input.h"

#include "ecdsa/preprocessing.hpp"
#include "ecdsa/sign.hpp"
#include "protocols/replicated/malicious_rep_mc.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/replicated/malicious_rep_prep.hpp"
#include "vm/input.hpp"
#include "vm/processor.hpp"
#include "vm/data_files.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/rep_prep.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/secret.hpp"
#include "parties/shared/shamir.hpp"
#include "parties/shared/malrep.hpp"
#include "parties/shared/rep.hpp"

#include <assert.h>

template<template<class U> class T>
void run(int argc, const char** argv)
{
    Bigint::initialize_thread();
    ez::EzOptionParser opt;
    EcdsaOptions opts(opt, argc, argv);
    opts.r_after_message |= is_same<T<P256Element>, AtlasShare<P256Element>>::value;
    Names N(opt, argc, argv,
            3 + is_same<T<P256Element>, Rep4Share<P256Element>>::value);
    int n_tuples = 1000;
    if (not opt.last_args.empty())
        n_tuples = atoi(opt.last_args[0]->c_str());
    CryptoPlayer P(N, "ecdsa");
    P256Element::init();
    typedef T<P256Element::Scalar> PShare;
    OnlineOptions::singleton.batch_size = 1;
    // synchronize
    Bundle<OctetStream> bundle(P);
    P.unchecked_broadcast(bundle);

    typename PShare::MacKeyType mac_key;
    PShare::read_or_generate_mac_key("", P, mac_key);

    Timer timer;
    timer.start();
    auto statistics = P.total_communication();
    ProtocolSet<typename T<P256Element::Scalar>::Honest> set(P, mac_key);
    PShare sk = set.protocol.get_random();
    cout << "Secret key generation took " << timer.elapsed() * 1e3 << " ms" << endl;
    (P.total_communication() - statistics).print(true);

    OnlineOptions::singleton.batch_size = (1 + PShare::Protocol::uses_triples) * n_tuples;
    DataPositions usage;
    typename PShare::TriplePrep prep(0, usage);
    typename PShare::MacCheck MCp(mac_key);
    ArithmeticProcessor _({}, 0);
    SubProcessor<PShare> proc(_, MCp, prep, P);

    bool prep_mult = not opt.isSet("-D");
    vector<EcTuple<T>> tuples;
    preprocessing<T>(tuples, n_tuples, sk, proc, opts);
//    check(tuples, sk, {}, P);
    sign_benchmark<T>(tuples, sk, MCp, P, opts, prep_mult ? 0 : &proc);
    P256Element::finish();
}
