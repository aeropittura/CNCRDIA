/*
 * he/offline/pairwise_machine.cpp
 *
 */

#include "he/offline/pairwise_machine.h"
#include "support/benchmarking.h"
#include "protocols/fake/fake_stuff.h"
#include "support/bundle.h"

#include "protocols/fake/fake_stuff.hpp"

PairwiseMachine::PairwiseMachine(Player& P) :
    P(P),
    other_pks(P.num_players(), {setup_p.params, 0}),
    pk(other_pks[P.my_num()]), sk(pk)
{
}

RealPairwiseMachine::RealPairwiseMachine(int argc, const char** argv) :
    MachineBase(argc, argv), PairwiseMachine(*new PlainPlayer(N, "pairwise"))
{
    init();
}

void RealPairwiseMachine::init()
{
    if (use_gf2n)
    {
        field_size = Gf2nShort::DEFAULT_LENGTH;
        Gf2nShort::initialize_field(field_size);
        setup_keys<P2Data>();
    }
    else
    {
        setup_keys<FftData>();
        Bigint p = setup_p.field_d.get_prime();
        Gfp::initialize_field(p);
        ofstream outf;
        if (output)
            Gfp::write_setup(get_prep_directory<FftData>(P));
    }

    for (int i = 0; i < nthreads; i++)
        if (use_gf2n)
            generators.push_back(new PairwiseGenerator<P2Data>(i, *this));
        else
            generators.push_back(new PairwiseGenerator<FftData>(i, *this));
}

template <>
PairwiseSetup<FftData>& PairwiseMachine::setup()
{
    return setup_p;
}

template <>
PairwiseSetup<P2Data>& PairwiseMachine::setup()
{
    return setup_2;
}

template <class FD>
void RealPairwiseMachine::setup_keys()
{
    auto& N = P;
    PairwiseSetup<FD>& s = setup<FD>();
    s.init(P, sec, field_size, extra_slack);
    if (output)
        write_mac_key(get_prep_directory<FD>(P), P.my_num(), P.num_players(), s.alphai);
    for (auto& x : other_pks)
        x = FHE_PK(s.params, s.field_d.get_prime());
    sk = FHE_SK(pk);
    PRNG G;
    G.re_seed();
    insecure("local key generation");
    key_gen(pk, sk, G);
    vector<OctetStream> os(N.num_players());
    pk.pack(os[N.my_num()]);
    P.broadcast_receive(os);
    for (int i = 0; i < N.num_players(); i++)
        if (i != N.my_num())
            other_pks[i].unpack(os[i]);
    set_mac_key(s.alphai);
    Share<typename FD::T>::MacCheck::setup(P);
}

template <class T>
void RealPairwiseMachine::set_mac_key(T alphai)
{
    typedef typename T::FD FD;
    auto& N = P;
    PairwiseSetup<FD>& s = setup<FD>();
    s.alphai = alphai;
    Share<typename FD::T>::set_mac_key(alphai);
    for (size_t i = 0; i < s.alpha.num_slots(); i++)
        s.alpha.set_element(i, alphai);
    insecure("MAC key generation");
    Ciphertext enc_alpha = pk.encrypt(s.alpha);
    vector<OctetStream> os;
    os.clear();
    os.resize(N.num_players());
    enc_alphas.resize(N.num_players(), pk);
    enc_alpha.pack(os[N.my_num()]);
    P.broadcast_receive(os);
    for (int i = 0; i < N.num_players(); i++)
        if (i != N.my_num())
            enc_alphas[i].unpack(os[i]);
    for (int i = 0; i < N.num_players(); i++)
        cout << "Player " << i << " has pk "
                << other_pks[i].a().get(0).get_constant().get_limb(0) << " ..."
                << endl;
}

void PairwiseMachine::pack(OctetStream& os) const
{
    os.store(other_pks);
    os.store(enc_alphas);
    sk.pack(os);
}

void PairwiseMachine::unpack(OctetStream& os)
{
    os.get_no_resize(other_pks);
    os.get(enc_alphas, {pk});
    sk.unpack(os);
}

void PairwiseMachine::check(Player& P) const
{
    Bundle<OctetStream> bundle(P);

    try
    {
        bundle.mine.store(other_pks);
        bundle.mine.store(enc_alphas);
    }
    catch (exception&)
    {
    }

    bundle.compare(P);
}

int PairwiseMachine::comp_sec()
{
    return NonInteractiveProof::comp_sec(sec);
}

template void RealPairwiseMachine::setup_keys<FftData>();
template void RealPairwiseMachine::setup_keys<P2Data>();
