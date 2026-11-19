/*
 * protocols/spdz2k/spdz2k.h
 *
 */

#pragma once

#include "protocols/spdz/spdz.h"

template<class T>
class Spdz2k : public SPDZ<T>
{
public:
    Spdz2k(Player& P) :
            SPDZ<T>(P)
    {
    }

    void exchange()
    {
        for (size_t i = 0;
                i < this->shares.size() + 2 * this->multiplicands.size(); i++)
            this->MC->set_random_element({});
        SPDZ<T>::exchange();
    }
};
