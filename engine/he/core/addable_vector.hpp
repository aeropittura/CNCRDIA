/*
 * he/core/addable_vector.hpp
 *
 */

#pragma once

#include "he/core/addable_vector.h"
#include "he/core/rq_element.h"
#include "he/core/fhe_keys.h"
#include "he/core/p2_data.h"

template<class T>
AddableVector<T> AddableVector<T>::mult_by_x_i(int j,
        const FHE_PK& pk) const
{
    int phi_m = this->size();
    assert(phi_m == pk.get_parameters().phi_m());
    AddableVector res(phi_m);
    for (int i = 0; i < phi_m; i++)
    {
        int k = j + i, s = 1;
        while (k >= phi_m)
        {
            k -= phi_m;
            s = -s;
        }
        if (s == 1)
        {
            res[k] = (*this)[i];
        }
        else
        {
            res[k] = -(*this)[i];
        }
    }
    return res;
}
