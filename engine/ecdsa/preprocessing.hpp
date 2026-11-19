/*
 * ecdsa/preprocessing.hpp
 *
 */

#pragma once

#include "ecdsa/p256_element.h"
#include "ecdsa/ecdsa_options.h"
#include "vm/data_files.h"
#include "protocols/common/replicated_prep.h"
#include "protocols/shamir/malicious_shamir_share.h"
#include "protocols/replicated/rep3_share.h"
#include "boolean/tinier_secret.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/tiny_mc.h"

#include "boolean/tinier_share_prep.hpp"
#include "boolean/ccd_secret.h"

template<template<class U> class T>
class EcTuple
{
public:
    T<P256Element::Scalar> a;
    T<P256Element::Scalar> b;
    P256Element::Scalar c;
    T<P256Element> secret_r;
    P256Element R;
};

template<template<class U> class T>
void preprocessing(vector<EcTuple<T>>& tuples, int buffer_size,
        T<P256Element::Scalar>& sk,
        SubProcessor<T<P256Element::Scalar>>& proc,
        EcdsaOptions opts)
{
    bool prep_mult = opts.prep_mult;
    if (prep_mult)
        proc.protocol.initialize_mult();

    Timer timer;
    timer.start();
    Player& P = proc.P;
    auto& prep = proc.DataF;
    size_t start = P.total_communication().sent;
    auto statistics = P.total_communication();
    auto& extra_player = P;

    auto& protocol = proc.protocol;
    auto& MCp = proc.MC;
    typedef T<typename P256Element::Scalar> PShare;
    typedef T<P256Element> CShare;
    vector<PShare> inverse_ks;
    vector<CShare> secret_rs;
    prep.buffer_triples();
    vector<PShare> bs, cs;
    for (int i = 0; i < buffer_size; i++)
    {
        PShare a, b, c;
        prep.get_three(DATA_TRIPLE, a, b, c);
        inverse_ks.push_back(a);
        bs.push_back(b);
        cs.push_back(c);
    }
    vector<P256Element::Scalar> cs_opened;
    MCp.POpen_Begin(cs_opened, cs, extra_player);
    if (opts.fewer_rounds)
        secret_rs.insert(secret_rs.begin(), bs.begin(), bs.end());
    else
    {
        MCp.POpen_End(cs_opened, cs, extra_player);
        for (int i = 0; i < buffer_size; i++)
            secret_rs.push_back(bs[i] / cs_opened[i]);
    }
    vector<P256Element> opened_rs(buffer_size);
    typename CShare::DirectOpener MCc(MCp.get_alphai());
    if (not opts.r_after_message)
        MCc.POpen_Begin(opened_rs, secret_rs, extra_player);
    if (prep_mult)
    {
        for (int i = 0; i < buffer_size; i++)
            protocol.prepare_mul(inverse_ks[i], sk);
        protocol.start_exchange();
    }
    if (opts.fewer_rounds)
        MCp.POpen_End(cs_opened, cs, extra_player);
    if (not opts.r_after_message)
    {
        MCc.POpen_End(opened_rs, secret_rs, extra_player);
        if (opts.fewer_rounds)
            for (int i = 0; i < buffer_size; i++)
                opened_rs[i] /= cs_opened[i];
    }
    if (prep_mult)
        protocol.stop_exchange();
    if (opts.check_open)
        MCc.Check(extra_player);
    if (opts.check_open or opts.check_beaver_open)
        MCp.Check(extra_player);
    for (int i = 0; i < buffer_size; i++)
    {
        tuples.push_back(
                { inverse_ks[i], prep_mult ? protocol.finalize_mul() : PShare(),
                        cs_opened[i], secret_rs[i], opened_rs[i] });
    }
    timer.stop();
    cout << "Generated " << buffer_size << " tuples in " << timer.elapsed()
            << " seconds, throughput " << buffer_size / timer.elapsed() << ", "
            << 1e-3 * (P.total_communication().sent - start) / buffer_size
            << " kbytes per tuple" << endl;
    (P.total_communication() - statistics).print(true);
}

template<template<class U> class T>
void check(vector<EcTuple<T>>& tuples, T<P256Element::Scalar> sk,
    P256Element::Scalar alphai, Player& P)
{
    typename T<P256Element::Scalar>::MacCheck MC(alphai);
    auto open_sk = MC.open(sk, P);
    for (auto& tuple : tuples)
    {
        auto inverse_k = MC.open(tuple.a, P);
        auto k = inverse_k.invert();
        assert(open_sk * inverse_k == MC.open(tuple.b, P));
        assert(tuple.R == k);
    }
    MC.Check(P);
}
