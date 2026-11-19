/*
 * protocols/common/share_vector.hpp
 *
 */

#pragma once

#include "protocols/common/share_vector.h"
#include "he/core/fft.h"

template<class U>
void ShareVector<U>::fft(const FftData& fftd)
{
    array<vector<modp>, 2> data;
    for (auto& share : *this)
    {
        data[0].push_back({share.get_share(), fftd.get_pr_d()});
        data[1].push_back({share.get_mac(), fftd.get_pr_d()});
    }

    for (auto& x : data)
    {
        if (fftd.get_twop() == 0)
            fft_iter2(x, fftd.phi_m(), fftd.get_root(0), fftd.get_pr_d());
        else
            fft_non_power_of_two(x, x, fftd);
    }

    for (int i = 0; i < fftd.phi_m(); i++)
    {
        typedef typename U::clear clear;
        (*this)[i] = {clear(data[0][i], fftd.get_pr_d()), clear(data[1][i], fftd.get_pr_d())};
    }
}
