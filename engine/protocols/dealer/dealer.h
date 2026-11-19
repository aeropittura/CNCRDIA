/*
 * protocols/dealer/dealer.h
 *
 */

#pragma once

#include "protocols/common/beaver.h"

template<class T>
class Dealer : public Beaver<T>
{
    SeededPrng G;

public:
    Dealer(Player& P) :
            Beaver<T>(P)
    {
    }

    T get_random()
    {
        if (T::real_shares(this->P))
            return G.get<T>();
        else
            return {};
    }

    vector<int> get_relevant_players()
    {
        return vector<int>(1, this->P.num_players() - 1);
    }
};
