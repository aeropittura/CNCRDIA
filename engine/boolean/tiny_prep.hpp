/*
 * boolean/tiny_prep.hpp
 *
 */

#pragma once

#include "boolean/tinier_share_prep.h"

#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/common/shuffle_sacrifice.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"

namespace GC
{

template<class T>
void TinierSharePrep<T>::initialize_real(Player& P)
{
    assert(real_triple_generator == 0);
    real_triple_generator = new typename T::WholeType::TripleGenerator(
            BaseMachine::fresh_ot_setup(P), P.N, -1,
            OnlineOptions::singleton.batch_size, 1, params,
            &P);
    real_triple_generator->multi_threaded = false;
}

template<class T>
void TinierSharePrep<T>::buffer_secret_triples()
{
    CODE_LOCATION
    auto& thread = ShareThread<SecretType>::s();
    auto& triple_generator = real_triple_generator;
    assert(triple_generator != 0);
    params.generate_bits = false;
    vector<array<T, 3>> triples;
    TripleShuffleSacrifice<T> sacrifice;
    size_t required;
    required = sacrifice.minimum_n_inputs_with_combining(
            BaseMachine::batch_size<T>(DATA_TRIPLE));
    triple_generator->set_batch_size(DIV_CEIL(required, 64));
    while (triples.size() < required)
    {
        triple_generator->generate_plain_triples();
        triple_generator->unlock();
        assert(triple_generator->plain_triples.size() != 0);
        for (size_t i = 0; i < triple_generator->plain_triples.size(); i++)
            triple_generator->value_bits[2].set_portion(i,
                    triple_generator->plain_triples[i][2]);
        triple_generator->run_multipliers({});
        assert(triple_generator->plain_triples.size() != 0);
        for (size_t i = 0; i < triple_generator->plain_triples.size(); i++)
        {
            int dl = SecretType::default_length;
            for (int j = 0; j < dl; j++)
            {
                triples.push_back({});
                for (int k = 0; k < 3; k++)
                {
                    auto& share = triples.back()[k];
                    share.set_share(
                            triple_generator->plain_triples.at(i).at(k).get_bit(
                                    j));
                    typename T::MacType mac;
                    mac = thread.MC->get_alphai() * share.get_share();
                    for (auto& multiplier : triple_generator->ot_multipliers)
                        mac += multiplier->macs.at(k).at(i * dl + j);
                    share.set_mac(mac);
                }
            }
        }
    }
    sacrifice.triple_sacrifice(triples, triples,
            *thread.P, thread.MC->get_part_opener());
    sacrifice.triple_combine(triples, triples, *thread.P,
            thread.MC->get_part_opener());
    for (auto& triple : triples)
        this->triples.push_back(triple);
}

} /* namespace GC */
