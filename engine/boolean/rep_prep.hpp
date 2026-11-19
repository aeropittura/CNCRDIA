/*
 * boolean/rep_prep.hpp
 *
 */

#pragma once

#include "boolean/rep_prep.h"
#include "boolean/share_thread.h"
#include "vm/online_options.h"

#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/common/replicated_prep.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "boolean/personal_prep.hpp"

namespace GC
{

template<class T>
RepPrep<T>::RepPrep(DataPositions& usage, int input_player) :
        PersonalPrep<T>(usage, input_player), protocol(0)
{
}

template<class T>
RepPrep<T>::~RepPrep()
{
    if (protocol)
        delete protocol;
}

template<class T>
void RepPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    if (this->protocol)
        return;

    this->protocol = new ReplicatedBase(protocol.P);
    this->P = &protocol.P;
}

template<class T>
void RepPrep<T>::buffer_triples()
{
    if (this->input_player != this->SECURE)
        return this->buffer_personal_triples();

    assert(protocol != 0);
    auto MC = ShareThread<T>::s().new_opener({});
    ThreadQueues* queues = 0;
    if (BaseMachine::has_singleton() and BaseMachine::thread_num == 0)
        queues = &BaseMachine::s().queues;
    shuffle_triple_generation(this->triples, protocol->P, *MC, 64, queues);
    MC->Check(protocol->P);
    delete MC;
}

template<class T>
void RepPrep<T>::buffer_bits()
{
    assert(this->protocol != 0);
    assert(this->protocol->P.num_players() == 3);
    for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
    {
        this->bits.push_back({});
        for (int j = 0; j < 2; j++)
            this->bits.back()[j] = this->protocol->shared_prngs[j].get_bit();
    }
}

template<class T>
void RepPrep<T>::buffer_inputs(int player)
{
    assert(this->protocol != 0);
    auto& protocol = *this->protocol;
    this->inputs.resize(protocol.P.num_players());
    if (player == protocol.P.my_num())
    {
        for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
        {
            InputTuple<T> tuple;
            for (int j = 0; j < 2; j++)
            {
                tuple.share[j].randomize(protocol.shared_prngs[j]);
                tuple.value += tuple.share[j];
            }
            this->inputs[player].push_back(tuple);
        }
    }
    else
    {
        for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
        {
            this->inputs[player].push_back({});
            int j = protocol.P.get_offset(player) - 1;
            this->inputs[player].back().share[j].randomize(protocol.shared_prngs[j]);
        }
    }
}

} /* namespace GC */
