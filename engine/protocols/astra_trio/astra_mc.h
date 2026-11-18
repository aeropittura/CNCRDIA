/*
 * protocols/astra_trio/astra_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class AstraMC : public MAC_Check_Base<T>
{
public:
    AstraMC(typename T::mac_key_type = {}, int = 0, int = 0)
    {
    }

    virtual typename T::open_type prepare_summand(const T& secret, int my_num);

    void exchange(const Player& P);

    AstraMC& get_part_MC()
    {
        return *this;
    }
};
