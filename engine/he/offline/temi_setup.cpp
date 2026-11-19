/*
 * he/offline/temi_setup.cpp
 *
 */

#include "he/offline/temi_setup.h"
#include "he/offline/pairwise_setup.h"
#include "he/core/ntl_subs.h"
#include "protocols/semi/hemi_options.h"

template<class FD>
TemiSetup<FD>::TemiSetup()
{
    this->params = FheParameters(0);
    this->pk = {this->params, 0};
    this->sk = {this->params, 0};
    this->calpha = this->params;
    this->params.set_matrix_dimension_from_options();
}

template<class FD>
void TemiSetup<FD>::secure_initialize(Player& P, MachineBase& machine,
        int plaintext_length, int, bool)
{
    ::secure_initialize(*this, P, machine, plaintext_length, 0, this->params);
}

template<class FD>
void TemiSetup<FD>::generate(Player& P, MachineBase&,
        int plaintext_length, int sec)
{
    generate_semi_setup(plaintext_length, sec, this->params, this->field_d,
        false, P.num_players());
    this->sk = {this->params, this->field_d.get_prime()};
    this->pk = {this->params, this->field_d.get_prime()};
}

template<class FD>
void TemiSetup<FD>::key_and_mac_generation(Player& P, MachineBase&, int,
        true_type)
{
    RqElement a(this->params);
    GlobalPrng GG(P);
    a.randomize(GG);
    SeededPrng G;
    auto sk = this->pk.sample_secret_key(G);
    this->sk.assign(sk);
    this->pk.partial_key_gen(sk, a, G);
    TreeSum<RqElement> ts;
    vector<RqElement> pks;
    pks.push_back(this->pk.b());
    ts.run(pks, P);
    this->pk.assign(this->pk.a(), pks[0]);
}

template class TemiSetup<FftData>;
template class TemiSetup<P2Data>;
