/*
 * protocols/shamir/malicious_shamir_mc.hpp
 *
 */

#pragma once

#include "protocols/shamir/malicious_shamir_mc.h"

#include "protocols/shamir/shamir_options.h"

template<class T>
MaliciousShamirOpener<T>::MaliciousShamirOpener()
{
    this->threshold = 2 * ShamirMachine::s().threshold;
}

template<class T>
void MaliciousShamirOpener<T>::initialize_open(const Player& P, int n)
{
    int threshold = ShamirMachine::s().threshold;
    if (reconstructions.empty())
    {
        reconstructions.resize(2 * threshold + 2);
        for (int i = threshold + 1; i <= 2 * threshold + 1; i++)
        {
            reconstructions[i] = ShamirOpener<T>::get_reconstruction(P, i);
        }
    }

    ShamirOpener<T>::initialize_open(P, n);
}

template<class T>
typename T::OpenType MaliciousShamirOpener<T>::finalize_raw()
{
    int threshold = ShamirMachine::s().threshold;
    shares.resize(2 * threshold + 1);
    for (size_t j = 0; j < shares.size(); j++)
        shares[j].unpack((*this->os)[this->player->get_player(j)]);
    return reconstruct(shares);
}

template<class T>
typename T::OpenType MaliciousShamirOpener<T>::reconstruct(
        const vector<OpenType>& shares)
{
    int threshold = ShamirMachine::s().threshold;
    assert(shares.size() == reconstructions.size() - 1);
    typename T::OpenType value = 0;
    for (int j = 0; j < threshold + 1; j++)
        value += shares[j] * reconstructions[threshold + 1][j];
    for (size_t j = threshold + 2; j <= shares.size(); j++)
    {
        typename T::OpenType check = 0;
        for (size_t k = 0; k < j; k++)
            check += shares[k] * reconstructions[j][k];
        if (check != value)
            throw MacFail("inconsistent Shamir secret sharing");
    }
    return value;
}
