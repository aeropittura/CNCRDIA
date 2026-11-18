/*
 * protocols/spdz/spdz.h
 *
 */

#pragma once

#include "protocols/semi/hemi.h"
#include "protocols/semi/semi_share.h"

#include <vector>
using namespace std;

template<class T> class SubProcessor;
template<class T> class Share;
class Player;

/**
 * SPDZ protocol
 */
template <class T>
class SPDZ : public MaybeHemi<T>
{
public:
    SPDZ(Player& P) : MaybeHemi<T>(P)
    {
    }

    int get_n_relevant_players()
    {
        return this->P.num_players();
    }
};
