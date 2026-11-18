/*
 * protocols/rep4/rep4_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class Rep4MC : public MAC_Check_Base<T>
{
    Hash check_hash, receive_hash;

public:
    Rep4MC(typename T::mac_key_type = {}, int = 0, int = 0)
    {
    }

    void exchange(const Player& P);
    void Check(const Player& P);

    Rep4MC& get_part_MC()
    {
        return *this;
    }
};
