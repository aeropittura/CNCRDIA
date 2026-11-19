/*
 * protocols/spdz/lowgear_key_gen.hpp
 *
 */

#pragma once

#include "protocols/spdz/lowgear_key_gen.h"
#include "he/core/rq_element.h"

#include "support/benchmarking.h"

#include "parties/shared/spdz.hpp"
#include "protocols/common/share_vector.hpp"
#include "protocols/spdz/mascot_prep.hpp"

template<int L>
LowGearKeyGenerator<L>::LowGearKeyGenerator(Player& P, PairwiseMachine& machine,
        FheParameters& params) :
        KeyGeneratorProtocol<1, L>(P, params), P(P), machine(machine)
{
}

template<int X, int L>
KeyGeneratorProtocol<X, L>::KeyGeneratorProtocol(Player& P, const FheParameters& params,
        int level) :
        P(P), params(params), fftd(params.FFTD().at(level)), usage(P)
{
    OpenType::initialize_field(params.FFTD().at(level).get_pr_d().pr, false);
    typename ShareType::MacKeyType alphai;

    auto& batch_size = OnlineOptions::singleton.batch_size;
    backup_batch_size = batch_size;
    batch_size = min(100, batch_size);

    if (OnlineOptions::singleton.live_prep)
    {
        prep = new MascotDabitOnlyPrep<ShareType>(0, usage);
        alphai = MascotDabitOnlyPrep<ShareType>::get_mac_key(P);
    }
    else
    {
        prep = new SubDataFiles<ShareType>(P.N,
                get_prep_sub_directory<ShareType>(P.num_players()), usage);
        read_mac_key(get_prep_sub_directory<ShareType>(P.num_players()), P.N, alphai);
    }

    MC = new MacCheck_<ShareType>(alphai);
    proc = new SubProcessor<ShareType>(*MC, *prep, P);

    MC->setup(P);
}

template<int X, int L>
KeyGeneratorProtocol<X, L>::~KeyGeneratorProtocol()
{
    MC->Check(P);

    if (OnlineOptions::singleton.verbose)
        usage.print_cost();

    delete proc;
    delete prep;
    delete MC;

    MC->teardown();
    MascotDabitOnlyPrep<ShareType>::teardown();

    OnlineOptions::singleton.batch_size = backup_batch_size;
    OpenType::reset();
}

template<int X, int L>
void KeyGeneratorProtocol<X, L>::input(vector<VectorType>& shares, const RqElement& secret)
{
    assert(secret.level() == 0);
    auto s = secret.get(0);
    s.change_rep(evaluation);
    auto& FFTD = s.get_fftd();
    auto& inputter = this->proc->input;
    inputter.reset_all(P);
    for (int i = 0; i < FFTD.num_slots(); i++)
        inputter.add_from_all(s.get_element(i));
    inputter.exchange();
    shares.clear();
    shares.resize(P.num_players());
    for (int i = 0; i < FFTD.num_slots(); i++)
        for (int j = 0; j < P.num_players(); j++)
            shares[j].push_back(inputter.finalize(j));
}

/**
 * Binomial secret generation from random bits
 */
template<int X, int L>
template<class T>
void KeyGeneratorProtocol<X, L>::binomial(VectorType& shares, T& prep)
{
    shares.resize(params.phi_m());
    RunningTimer timer, total;
    for (int i = 0; i < params.phi_m(); i++)
    {
#ifdef VERBOSE
        if (timer.elapsed() > 10)
        {
            cerr << i << "/" << params.phi_m() << ", throughput " <<
                    i / total.elapsed() << endl;
            timer.reset();
        }
#endif

        auto& share = shares[i];
        share = {};
        for (int i = 0; i < params.get_dg().get_new_hope_b(); i++)
        {
            share += prep.get_bit();
            share -= prep.get_bit();
        }
    }
    shares.fft(fftd);
}

template<int X, int L>
template<class T>
void KeyGeneratorProtocol<X, L>::secret_key(VectorType& shares, T& prep)
{
    cerr << "Generate secret key by ";
    cerr << "binomial" << endl;
    binomial(shares, prep);
}

template<int X, int L>
typename KeyGeneratorProtocol<X, L>::VectorType KeyGeneratorProtocol<X, L>::schur_product(
        const VectorType& x, const VectorType& y)
{
    VectorType res;
    assert(x.size() == y.size());
    auto& protocol = proc->protocol;
    protocol.initialize_mult();
    for (size_t i = 0; i < x.size(); i++)
        protocol.prepare_mul(x[i], y[i]);
    protocol.exchange();
    for (size_t i = 0; i < x.size(); i++)
        res.push_back(protocol.finalize_mul());
    return res;
}

template<int X, int L>
void KeyGeneratorProtocol<X, L>::output_to(int player, vector<OpenType>& opened,
        vector<ShareType>& shares)
{
    PrivateOutput<ShareType> po(*proc);
    for (auto& share : shares)
        po.prepare_sending(share, player);
    po.exchange();
    opened.resize(shares.size());
    for (auto& x : opened)
        x = po.finalize(player);
}

template<int L>
void LowGearKeyGenerator<L>::generate_keys(FheParameters& params)
{
    RunningTimer timer;
    auto& pk = machine.pk;

    GlobalPrng global_prng(P);
    auto& FFTD = pk.get_parameters().FFTD()[0];

    for (int i = 0; i < P.num_players(); i++)
    {
        VectorType sk;
        this->secret_key(sk, *this);
        vector<OpenType> open_sk;
        this->output_to(i, open_sk, sk);
        if (P.my_num() == i)
            machine.sk.assign(RingElement(FFTD, evaluation, open_sk));
        VectorType e0;
        this->binomial(e0, *this);
        AddableVector<OpenType> a0(pk.get_parameters().phi_m());
        a0.randomize(global_prng);
        vector<OpenType> b0;
        assert(machine.sk.p() != 0);
        this->MC->POpen(b0, sk * a0 + e0 * machine.sk.p(), P);
        machine.other_pks[i] = FHE_PK(params, machine.sk.p());
        machine.other_pks[i].assign(RingElement(FFTD, evaluation, a0),
                RingElement(FFTD, evaluation, b0));
    }

    this->MC->Check(P);

    cerr << "Key generation took " << timer.elapsed() << " seconds" << endl;
}

template<int L>
template<class FD>
void LowGearKeyGenerator<L>::run(PairwiseSetup<FD>& setup)
{
    CODE_LOCATION
    generate_keys(setup.params);
    machine.sk.check(machine.pk, setup.field_d);

    RunningTimer timer;

    typename FD::T mac_key;

    bool fresh = setup.alphai == 0;

    if (fresh)
        mac_key = SeededPrng().get<typename FD::T>();
    else
        mac_key = setup.alphai;

    PairwiseGenerator<FD> generator(0, machine, &P);
    map<string, Timer> timers;
    MultiEncCommit<FD> EC(P, machine.other_pks, setup.field_d,
            timers, machine, generator, true);
    assert(EC.proof.get_diagonal());
    vector<Plaintext_<FD>> m(EC.proof.U, setup.field_d);
    for (auto& mm : m)
        mm.assign_constant(mac_key);

    AddableVector<Ciphertext> C;
    OctetStream ciphertexts, cleartexts;
    EC.get_proof().set_session_id(P);
    EC.generate_proof(C, m, ciphertexts, cleartexts);

    AddableVector<Ciphertext> others_ciphertexts;
    others_ciphertexts.resize(EC.proof.U, machine.pk.get_parameters());
    Verifier<FD> verifier(EC.proof, setup.field_d);
    verifier.nizk_pok(others_ciphertexts, ciphertexts,
            cleartexts, machine.pk, P.my_num());

    machine.enc_alphas.clear();
    for (int i = 0; i < P.num_players(); i++)
        machine.enc_alphas.push_back(machine.other_pks[i]);

    for (int i = 1; i < P.num_players(); i++)
    {
        int player = P.get_player(-i);
#ifdef VERBOSE_HE
        cerr << "Sending proof with " << 1e-9 * ciphertexts.get_length() << "+"
                << 1e-9 * cleartexts.get_length() << " GB" << endl;
#endif
        timers["Sending"].start();
        P.pass_around(ciphertexts);
        P.pass_around(cleartexts);
        timers["Sending"].stop();
#ifdef VERBOSE_HE
        cerr << "Checking proof of player " << i << endl;
#endif
        timers["Verifying"].start();
        verifier.nizk_pok(others_ciphertexts, ciphertexts,
                cleartexts, machine.other_pks[player], player);
        timers["Verifying"].stop();
        machine.enc_alphas.at(player) = others_ciphertexts.at(0);
    }

    setup.set_alphai(mac_key);
    machine.enc_alphas.at(P.my_num()) = C.at(0);

    auto test = machine.sk.decrypt(C[0], setup.field_d);
    for (int i = 0; i < setup.field_d.num_slots(); i++)
        assert(test.element(i) == mac_key);

    cerr << "MAC key generation took " << timer.elapsed() << " seconds" << endl;
}
