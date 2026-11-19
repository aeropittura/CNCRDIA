/*
 * protocols/rep4/rep4_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class Rep4Opener : public MacCheckBase<T>
{
    Hash check_hash, receive_hash;

public:
    Rep4Opener(typename T::MacKeyType = {}, int = 0, int = 0)
    {
    }

    void exchange(const Player& P);
    void Check(const Player& P);

    Rep4Opener& get_part_opener()
    {
        return *this;
    }
};
