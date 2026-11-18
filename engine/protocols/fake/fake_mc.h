/*
 * protocols/fake/fake_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class FakeMC : public MAC_Check_Base<T>
{
public:
    FakeMC(typename T::mac_key_type, int = 0, int = 0)
    {
    }

    void exchange(const Player&)
    {
        for (auto& x : this->secrets)
            this->values.push_back(x);
    }

    FakeMC& get_part_MC()
    {
        return *this;
    }
};
