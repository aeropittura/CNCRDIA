/*
 * protocols/shamir/malicious_shamir_po.hpp
 *
 */

#pragma once

#include "protocols/shamir/malicious_shamir_po.h"

template<class T>
MaliciousShamirPo<T>::MaliciousShamirPo(Player& P) :
        P(P), shares(P.num_players())
{
    MC.initialize_open(P);
}

template<class T>
int MaliciousShamirPo<T>::n_relevant_players()
{
    return MC.get_threshold() + 1;
}

template<class T>
void MaliciousShamirPo<T>::prepare_sending(const T& secret, int)
{
    secret.pack(to_send);
}

template<class T>
void MaliciousShamirPo<T>::send(int player)
{
    if (P.my_num() < n_relevant_players())
        P.send_to(player, to_send);
}

template<class T>
void MaliciousShamirPo<T>::receive()
{
    to_receive.resize(n_relevant_players());
    for (int i = 0; i < n_relevant_players(); i++)
        if (i != P.my_num())
            P.receive_player(i, to_receive[i]);
}

template<class T>
typename T::clear MaliciousShamirPo<T>::finalize(const T& secret)
{
    shares.resize(n_relevant_players());
    for (int i = 0; i < n_relevant_players(); i++)
    {
        if (i == P.my_num())
            shares[0] = secret;
        else
            shares[P.get_offset(i)].unpack(to_receive[i]);
    }

    return MC.reconstruct(shares);
}
