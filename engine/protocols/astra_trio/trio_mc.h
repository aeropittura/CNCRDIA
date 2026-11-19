/*
 * protocols/astra_trio/trio_mc.h
 *
 */

#pragma once

#include "protocols/astra_trio/astra_mc.h"

template<class T>
class TrioOpener : public AstraOpener<T>
{
public:
    TrioOpener(typename T::MacKeyType = {}, int = 0, int = 0)
    {
    }

    typename T::OpenType prepare_summand(const T& secret, int my_num)
    {
        return secret[my_num - 1];
    }

    TrioOpener& get_part_opener()
    {
        return *this;
    }
};
