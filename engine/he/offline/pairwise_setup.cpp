/*
 * he/offline/pairwise_setup.cpp
 *
 */

#include "he/offline/pairwise_setup.h"
#include "he/core/noise_bounds.h"
#include "he/core/ntl_subs.h"
#include "numeric/setup.h"
#include "he/offline/proof.h"
#include "he/offline/pairwise_machine.h"
#include "he/offline/temi_setup.h"
#include "support/commit.h"
#include "support/bundle.h"
#include "vm/online_options.h"
#include "protocols/spdz/lowgear_key_gen.h"

#include "protocols/common/share.hpp"
#include "protocols/common/mac_key.hpp"

template <class FD>
void PairwiseSetup<FD>::init(const Player& P, int sec, int plaintext_length,
    int& extra_slack)
{
    cout << "Finding parameters for security " << sec << " and field size ~2^"
            << plaintext_length << endl;
    PRNG G;
    G.re_seed();

    OctetStream o;
    if (P.my_num() == 0)
    {
        extra_slack =
            generate_semi_setup(plaintext_length, sec, params, field_d, true);
        params.pack(o);
        field_d.pack(o);
        o.store(extra_slack);
        P.send_all(o);
    }
    else
    {
        P.receive_player(0, o);
        params.unpack(o);
        field_d.unpack(o);
        field_d.initialize_field();
        o.get(extra_slack);
    }

    alpha = field_d;
    alphai.randomize(G);
    alpha.assign_constant(alphai);
}

template <class FD>
void PairwiseSetup<FD>::secure_initialize(Player& P, PairwiseMachine& machine,
        int plaintext_length, int sec, bool read_only)
{
    ::secure_initialize(*this, P, machine, plaintext_length, sec, params, read_only);
    alpha = field_d;
    machine.sk = FHE_SK(params, field_d.get_prime());
    for (auto& pk : machine.other_pks)
        pk = FHE_PK(params, field_d.get_prime());
}

template <class T, class U>
void secure_initialize(T& setup, Player& P, U& machine,
        int plaintext_length, int sec,
        FheParameters& params, bool read_only)
{
    assert(sec >= 0);
    machine.sec = sec;
    params.set_min_sec(sec);
    string filename = PREP_DIR + T::name() + "-"
            + to_string(plaintext_length) + "-" + to_string(sec) + "-"
            + to_string(params.secp()) + "-"
            + to_string(machine.comp_sec()) + "-"
            + to_string(params.get_matrix_dimension()) + "-"
            + OnlineOptions::singleton.prime.get_str() + "-"
            + to_string(CowGearOptions::singleton.top_gear()) + "-P"
            + to_string(P.my_num()) + "-" + to_string(P.num_players());
    string reason;
    auto base_setup = setup;

    try
    {
        OctetStream os;
        os.input(filename);
        os.get(machine.extra_slack);
        setup.unpack(os);
    }
    catch (exception& e)
    {
        reason = e.what();
    }

    try
    {
        setup.check(P, machine);
    }
    catch (MismatchAmongParties& e)
    {
        if (reason.empty())
            reason = e.what();
    }

    if (not reason.empty())
    {
        if (OnlineOptions::singleton.has_option("expect_setup") or read_only)
            throw runtime_error("error in setup: " + reason);

        if (OnlineOptions::singleton.verbose)
            cerr << "Generating parameters for security " << sec
                    << " and field size ~2^" << plaintext_length
                    << " because no suitable material "
                            "from a previous run was found (" << reason << ")"
                    << endl;
        setup = base_setup;
        setup.generate(P, machine, plaintext_length, sec);
        setup.check(P, machine);
        OctetStream os;
        os.store(machine.extra_slack);
        setup.pack(os);
        ofstream file(filename);
        os.output(file);
    }

    if (OnlineOptions::singleton.has_option("verbose_he"))
    {
        cerr << "Ciphertext length: " << params.p0().num_bits();
        for (size_t i = 1; i < params.FFTD().size(); i++)
            cerr << "+" << params.FFTD()[i].get_prime().num_bits();
        cerr << " (" << DIV_CEIL(params.p0().num_bits(), 64);
        for (size_t i = 1; i < params.FFTD().size(); i++)
            cerr << "+" << DIV_CEIL(params.FFTD()[i].get_prime().num_bits(), 64);
        cerr << " limbs)";
        cerr << endl;
        cerr << "Number of slots: " << params.phi_m() << endl;
    }
}

template <class FD>
void PairwiseSetup<FD>::generate(Player&, MachineBase& machine,
        int plaintext_length, int sec)
{
    machine.extra_slack = generate_semi_setup(plaintext_length, sec, params,
            field_d, true);
}

template<class FD>
void PairwiseSetup<FD>::pack(OctetStream& os) const
{
    params.pack(os);
    field_d.pack(os);
    alpha.pack(os);
    alphai.pack(os);
}

template<class FD>
void PairwiseSetup<FD>::unpack(OctetStream& os)
{
    params.unpack(os);
    field_d.unpack(os);
    field_d.initialize_field();
    alpha.unpack(os);
    alphai.unpack(os);
}

template <class FD>
void PairwiseSetup<FD>::check(Player& P, PairwiseMachine& machine)
{
    Bundle<OctetStream> bundle(P);
    bundle.mine.store(machine.extra_slack);
    params.pack(bundle.mine);
    field_d.hash(bundle.mine);
    bundle.compare(P);
}

template <class FD>
void PairwiseSetup<FD>::covert_key_generation(Player& P,
        PairwiseMachine& machine, int num_runs)
{
    CODE_LOCATION
    vector<SeededPrng> G(num_runs);
    vector<AllCommitments> commits(num_runs, P);
    vector<FheKeyPair> my_keys(num_runs, {params, field_d.get_prime()});
    Bundle<OctetStream> pks(P);

    for (int i = 0; i < num_runs; i++)
    {
        my_keys[i].generate(G[i]);
        my_keys[i].pk.pack(pks.mine);
        commits[i].commit({SEED_SIZE, G[i].get_seed()});
    }

    P.broadcast_receive(pks);
    int challenge = GlobalPrng(P).get_uint(num_runs);
    machine.sk = my_keys[challenge].sk;
    machine.pk = my_keys[challenge].pk;

    for (int i = 0; i < num_runs; i++)
        if (i != challenge)
            commits[i].open({SEED_SIZE, G[i].get_seed()});

    for (int i = 0; i < num_runs; i++)
    {
         for (int j = 0; j < P.num_players(); j++)
            if (j != P.my_num())
            {
                FHE_PK pk(params);
                pk.unpack(pks[j]);
                if (i == challenge)
                    machine.other_pks[j] = pk;
                else
                {
                    FheKeyPair pair(params, field_d.get_prime());
                    PRNG prng(commits[i].messages[j]);
                    pair.generate(prng);
                    if (pair.pk != pk)
                        throw BadKeygen("covert pairwise key generation");
                }
            }
    }
}

template <class FD>
void PairwiseSetup<FD>::covert_mac_generation(Player& P,
        PairwiseMachine& machine, int num_runs)
{
    vector<const FHE_PK*> pks;
    for (auto& pk : machine.other_pks)
        pks.push_back(&pk);
    covert_generation(alpha, machine.enc_alphas, pks, &P, num_runs, Diagonal);
    alphai = alpha.element(0);
}

template <class FD>
void PairwiseSetup<FD>::key_and_mac_generation(Player& P,
        PairwiseMachine& machine, int num_runs, true_type)
{
    covert_key_generation(P, machine, num_runs);
    covert_mac_generation(P, machine, num_runs);
}

template <class FD>
void PairwiseSetup<FD>::set_alphai(T alphai)
{
    this->alphai = alphai;
    alpha.assign_constant(alphai);
}

template class PairwiseSetup<FftData>;
template class PairwiseSetup<P2Data>;

template void secure_initialize(PartSetup<FftData>&, Player&, MachineBase&, int, int, FheParameters& params, bool);
template void secure_initialize(PartSetup<P2Data>&, Player&, MachineBase&, int, int, FheParameters& params, bool);

template void secure_initialize(TemiSetup<FftData>&, Player&, MachineBase&, int, int, FheParameters& params, bool);
template void secure_initialize(TemiSetup<P2Data>&, Player&, MachineBase&, int, int, FheParameters& params, bool);
