/*
 * protocols/astra_trio/astra_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
class AstraOpener : public MacCheckBase<T>
{
public:
    AstraOpener(typename T::MacKeyType = {}, int = 0, int = 0)
    {
    }

    virtual typename T::OpenType prepare_summand(const T& secret, int my_num);

    void exchange(const Player& P);

    AstraOpener& get_part_opener()
    {
        return *this;
    }
};
