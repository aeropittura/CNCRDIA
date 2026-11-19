/*
 * he/offline/data_setup.cpp
 *
 */

#include "he/offline/data_setup.h"
#include "he/offline/dist_key_gen.h"
#include "protocols/fake/fake_stuff.h"
#include "he/core/ntl_subs.h"
#include "support/benchmarking.h"
#include "support/bundle.h"
#include "he/offline/pairwise_setup.h"
#include "he/offline/proof.h"
#include "he/offline/simple_machine.h"
#include "he/offline/pairwise_machine.h"

#include <iostream>
using namespace std;

#include "protocols/fake/fake_stuff.hpp"

template <class FD>
PartSetup<FD>::PartSetup() :
    pk(params, 0), sk(params, 0), calpha(params)
{
}

DataSetup::DataSetup()
{
}

DataSetup& DataSetup::operator=(const DataSetup& other)
{
  setup_p = other.setup_p;
  setup_2 = other.setup_2;
  return *this;
}

template <class FD>
void PartSetup<FD>::generate_setup(int n_parties, int plaintext_length, int sec,
    int slack, bool round_up)
{
  params.set_min_sec(sec);
  Parameters(n_parties, plaintext_length, sec, slack, round_up).generate_setup(
      params, field_d);
  pk = FHE_PK(params, field_d.get_prime());
  sk = FHE_SK(params, field_d.get_prime());
  calpha = Ciphertext(params);
}

template <class FD>
void PartSetup<FD>::fake(vector<FHE_SK>& sks, vector<T>& alphais,
        int nplayers, bool distributed, bool check_security)
{
  if (check_security)
    insecure("global key generation");
  if (distributed)
      cout << "Faking distributed key generation" << endl;
  else
      cout << "Faking key generation with extra noise" << endl;
  PRNG G;
  G.re_seed();
  pk = FHE_PK(params, field_d.get_prime());
  FHE_SK sk(params, field_d.get_prime());
  calpha = Ciphertext(params);
  sks.resize(nplayers, pk);
  alphais.resize(nplayers);

  if (distributed)
      DistKeyGenerator::fake(pk, sks, field_d.get_prime(), nplayers);
  else
  {
      RqElement sk = FHE_SK(pk).s();
      for (int i = 0; i < nplayers; i++)
      {
          RqElement ski = pk.sample_secret_key(G);
          sks[i].assign(ski);
          sk += ski;
      }
      pk.key_gen(sk, G, nplayers);
  }

  for (int i = 0; i < nplayers; i++)
    {
      Plaintext_<FD> m(field_d);
      m.randomize(G,Diagonal);
      Ciphertext calphai = pk.encrypt(m);
      calpha += calphai;
      alphais[i] = m.element(0);
    }
}

template <class FD>
void PartSetup<FD>::fake(vector<PartSetup<FD> >& setups, int nplayers,
        bool distributed, bool check_security)
{
    vector<FHE_SK> sks;
    vector<T> alphais;
    fake(sks, alphais, nplayers, distributed, check_security);
    setups.clear();
    setups.resize(nplayers, *this);
    for (int i = 0; i < nplayers; i++)
    {
        setups[i].sk = sks[i];
        setups[i].alphai = alphais[i];
    }
}

template <class FD>
void PartSetup<FD>::insecure_debug_keys(vector<PartSetup<FD> >& setups, int nplayers, bool simple_pk)
{
    cout << "generating INSECURE keys for debugging" << endl;
    setups.clear();
    RqElement zero(params, evaluation, evaluation),
            one(params, evaluation, evaluation);
    zero.assign_zero();
    one.assign_one();
    PRNG G;
    G.re_seed();
    if (simple_pk)
        pk.assign(zero, zero, zero, zero - one);
    else
        pk.key_gen(one, G, nplayers);
    setups.resize(nplayers, *this);
    setups[0].sk.assign(one);
    for (int i = 1; i < nplayers; i++)
        setups[i].sk.assign(zero);
}

template<class FD>
void PartSetup<FD>::output(Names& N)
{
    // Write outputs to file
    string dir = get_prep_sub_directory<Share<typename FD::T>>(N.num_players());
    field_d.write_setup(dir);
    write_mac_key(dir, N.my_num(), N.num_players(), alphai);
}

template <class FD>
void PartSetup<FD>::pack(OctetStream& os)
{
    os.append((Octet*)"PARTSETU", 8);
    params.pack(os);
    field_d.pack(os);
    pk.pack(os);
    sk.pack(os);
    calpha.pack(os);
    alphai.pack(os);
}

template <class FD>
void PartSetup<FD>::unpack(OctetStream& os)
{
    char tag[8];
    os.consume((Octet*) tag, 8);
    if (memcmp(tag, "PARTSETU", 8))
      throw runtime_error("invalid serialization of setup");
    params.unpack(os);
    field_d.unpack(os);
    pk = {params, field_d};
    sk = pk;
    calpha = params;
    pk.unpack(os);
    sk.unpack(os);
    calpha.unpack(os);
    initialize_field();
    alphai.unpack(os);
}

template <>
void PartSetup<FftData>::initialize_field()
{
    Gfp::initialize_field(field_d.get_prime());
}

template <>
void PartSetup<P2Data>::initialize_field()
{
}

template <class FD>
void PartSetup<FD>::check() const
{
    sk.check(params, pk, field_d.get_prime());
}

template <class FD>
bool PartSetup<FD>::operator!=(const PartSetup<FD>& other)
{
    if (params != other.params or field_d != other.field_d or pk != other.pk
            or sk != other.sk or calpha != other.calpha
            or alphai != other.alphai)
        return true;
    else
        return false;
}

template<class FD>
void PartSetup<FD>::secure_initialize(Player& P, MachineBase& machine,
    int plaintext_length, int sec, bool read_only)
{
    ::secure_initialize(*this, P, machine, plaintext_length, sec, params, read_only);
}

template<class FD>
void PartSetup<FD>::generate(Player& P, MachineBase&, int plaintext_length,
        int sec)
{
    generate_setup(P.num_players(), plaintext_length, sec,
            INTERACTIVE_SPDZ1_SLACK, false);
}

template<class FD>
void PartSetup<FD>::check(Player& P, MachineBase& machine)
{
    Bundle<OctetStream> bundle(P);
    bundle.mine.store(machine.extra_slack);
    auto& os = bundle.mine;
    params.pack(os);
    field_d.hash(os);
    pk.pack(os);
    calpha.pack(os);
    bundle.compare(P);
}

template<class FD>
void PartSetup<FD>::covert_key_generation(Player& P, MachineBase&, int num_runs)
{
    run_gen_protocol(pk, sk, P, num_runs, false);
}

template<class FD>
void PartSetup<FD>::covert_mac_generation(Player& P, MachineBase&, int num_runs)
{
    generate_mac_key(alphai, calpha, field_d, pk, P,
            num_runs);
}

template<class FD>
void PartSetup<FD>::key_and_mac_generation(Player& P,
        MachineBase& machine, int num_runs, true_type)
{
    covert_key_generation(P, machine, num_runs);
    covert_mac_generation(P, machine, num_runs);
}

template class PartSetup<FftData>;
template class PartSetup<P2Data>;
