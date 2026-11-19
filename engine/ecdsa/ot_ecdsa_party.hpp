/*
 * ecdsa/ot_ecdsa_party.hpp
 *
 */

#pragma once

#include "net/server.h"
#include "net/crypto_player.h"
#include "numeric/gfp.h"
#include "ecdsa/p256_element.h"
#include "protocols/semi/semi_share.h"
#include "protocols/spdz/spdz.h"
#include "vm/base_machine.h"

#include "ecdsa/preprocessing.hpp"
#include "ecdsa/sign.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/semi/hemi.hpp"
#include "vm/processor.hpp"
#include "vm/data_files.hpp"
#include "vm/input.hpp"
#include "boolean/tiny_prep.hpp"
#include "boolean/vector_protocol.hpp"
#include "boolean/ccd_prep.hpp"

#include <assert.h>

template<template<class U> class T>
void run(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    EcdsaOptions opts(opt, argc, argv);
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Use SimpleOT instead of OT extension", // Help description.
            "-S", // Flag token.
            "--simple-ot" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Don't check correlation in OT extension (only relevant with MASCOT)", // Help description.
            "-U", // Flag token.
            "--unchecked-correlation" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Fewer rounds for authentication (only relevant with MASCOT)", // Help description.
            "-A", // Flag token.
            "--auth-fewer-rounds" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Use Fiat-Shamir for amplification (only relevant with MASCOT)", // Help description.
            "-H", // Flag token.
            "--fiat-shamir" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Skip sacrifice (only relevant with MASCOT)", // Help description.
            "-E", // Flag token.
            "--embrace-life" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "No MACs (only relevant with MASCOT; implies skipping MAC checks)", // Help description.
            "-M", // Flag token.
            "--no-macs" // Flag token.
    );

    Names N(opt, argc, argv, 2);
    int n_tuples = 1000;
    if (not opt.last_args.empty())
        n_tuples = atoi(opt.last_args[0]->c_str());
    PlainPlayer P(N, "ecdsa");
    P256Element::init();
    P256Element::Scalar::next::initialize_field(P256Element::Scalar::pr(), false);

    typedef T<P256Element::Scalar> PShare;
    DataPositions usage;

    auto keyp = PShare::LivePrep::get_mac_key(P);

    PShare::MacCheck::setup(P);
    T<P256Element>::MacCheck::setup(P);

    OnlineOptions::singleton.batch_size = 1;
    typename PShare::DirectOpener MCp(keyp);
    ArithmeticProcessor _({}, 0);
    typename PShare::TriplePrep sk_prep(0, usage);
    SubProcessor<PShare> sk_processor(_, MCp, sk_prep, P);
    PShare sk, __;
    // synchronize
    Bundle<OctetStream> bundle(P);
    P.unchecked_broadcast(bundle);
    Timer timer;
    timer.start();
    auto statistics = P.total_communication();
    sk_prep.get_two(DATA_INVERSE, sk, __);
    cout << "Secret key generation took " << timer.elapsed() * 1e3 << " ms" << endl;
    (P.total_communication() - statistics).print(true);

    OnlineOptions::singleton.batch_size = (1 + PShare::Protocol::uses_triples) * n_tuples;
    typename PShare::TriplePrep prep(0, usage);
    prep.params.correlation_check &= not opt.isSet("-U");
    prep.params.fewer_rounds = opt.isSet("-A");
    prep.params.fiat_shamir = opt.isSet("-H");
    prep.params.check = not opt.isSet("-E");
    prep.params.generate_macs = not opt.isSet("-M");
    opts.check_beaver_open &= prep.params.generate_macs;
    opts.check_open &= prep.params.generate_macs;
    SubProcessor<PShare> proc(_, MCp, prep, P);
    typename PShare::PrepType::DirectOpener MCpp(keyp);
    prep.triple_generator->MC = &MCpp;

    bool prep_mult = not opt.isSet("-D");
    prep.params.use_extension = not opt.isSet("-S");
    vector<EcTuple<T>> tuples;
    preprocessing(tuples, n_tuples, sk, proc, opts);
    //check(tuples, sk, keyp, P);
    sign_benchmark(tuples, sk, MCp, P, opts, prep_mult ? 0 : &proc);

    PShare::MacCheck::teardown();
    T<P256Element>::MacCheck::teardown();
    P256Element::finish();
}
