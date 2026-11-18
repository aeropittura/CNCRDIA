/*
 * protocols/astra_trio/trio_mc.h
 *
 */

#pragma once

#include "protocols/astra_trio/astra_mc.h"

template<class T>
class TrioMC : public AstraMC<T>
{
public:
    TrioMC(typename T::mac_key_type = {}, int = 0, int = 0)
    {
    }

    typename T::open_type prepare_summand(const T& secret, int my_num)
    {
        return secret[my_num - 1];
    }

    TrioMC& get_part_MC()
    {
        return *this;
    }
};
