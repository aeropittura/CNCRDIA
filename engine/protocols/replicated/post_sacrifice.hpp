/*
 * protocols/replicated/post_sacrifice.hpp
 *
 */

#pragma once

#include "protocols/replicated/post_sacrifice.h"

template<class T>
PostSacrifice<T>::PostSacrifice(Player& P) :
        internal(P), randomizer(P), P(P)
{
    results.reserve(OnlineOptions::singleton.batch_size);
}

template<class T>
PostSacrifice<T>::~PostSacrifice()
{
    check();
}

template<class T>
Player& PostSacrifice<T>::branch()
{
    return P;
}

template<class T>
void PostSacrifice<T>::initialize_mult()
{
    // throw away unused operands
    operands.resize(results.size());
    if ((int) results.size() >= OnlineOptions::singleton.batch_size)
        check();
    internal.initialize_mult();
}

template<class T>
void PostSacrifice<T>::prepare_mul(const T& x, const T& y, int n)
{
    (void) n;
    operands.push_back({{x, y}});
    internal.prepare_mul(x, y);
}

template<class T>
T PostSacrifice<T>::finalize_mul(int n)
{
    (void) n;
    auto res = internal.finalize_mul();
    results.push_back(res);
    return res;
}

template<class T>
void PostSacrifice<T>::check()
{
    int buffer_size = results.size();
    if (buffer_size == 0)
        return;

    CODE_LOCATION
    vector<array<PrepType, 5>> tuples;
    tuples.reserve(buffer_size);
    auto& honest_protocol = internal;
    honest_protocol.initialize_mult();
    for (int i = 0; i < buffer_size; i++)
    {
        tuples.push_back({});
        auto& tuple = tuples.back();
        tuple[0] = operands[i][0];
        tuple[2] = operands[i][1];
        tuple[3] = results[i];
        tuple[1] = randomizer.get_random();
        honest_protocol.prepare_mul(tuple[1], tuple[2]);
    }
    honest_protocol.exchange();
    for (int i = 0; i < buffer_size; i++)
    {
        tuples[i][4] = honest_protocol.finalize_mul();
    }
    sacrifice<PrepType, typename T::random_type>(tuples, P);
    operands.erase(operands.begin(), operands.begin() + buffer_size);
    results.clear();
}
