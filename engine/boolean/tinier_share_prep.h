/*
 * boolean/tinier_share_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"
#include "ot/n_party_triple_generator.h"
#include "boolean/share_thread.h"
#include "boolean/personal_prep.h"

namespace GC
{

template<class T> class TinierPrep;
template<class T> class TinierSecret;

template<class T>
class TinierSharePrep : public PersonalPrep<T>
{
    typename T::TripleGenerator* triple_generator;
    typename T::WholeType::TripleGenerator* real_triple_generator;
    MascotParameters params;

    typedef typename T::WholeType SecretType;

    void buffer_triples();
    void buffer_squares() { throw NotImplemented(); }
    void buffer_bits();
    void buffer_inverses() { throw NotImplemented(); }

    void buffer_inputs(int player);

    void buffer_secret_triples();

    void initialize_real(Player& P);

public:
    static typename T::MacKeyType get_mac_key(Player& P)
    {
        return T::TripleGenerator::get_mac_key(P);
    }

    TinierSharePrep(DataPositions& usage, int input_player =
            PersonalPrep<T>::SECURE);
    TinierSharePrep(SubProcessor<T>*, DataPositions& usage);
    ~TinierSharePrep();

    void set_protocol(typename T::Protocol& protocol);
};

}
