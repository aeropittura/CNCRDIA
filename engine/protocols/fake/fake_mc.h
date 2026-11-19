/*
 * protocols/fake/fake_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class FakeOpener : public MacCheckBase<T>
{
public:
    FakeOpener(typename T::MacKeyType, int = 0, int = 0)
    {
    }

    void exchange(const Player&)
    {
        for (auto& x : this->secrets)
            this->values.push_back(x);
    }

    FakeOpener& get_part_opener()
    {
        return *this;
    }
};
