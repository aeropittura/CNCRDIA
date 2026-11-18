/*
 * protocols/common/share_vector.h
 *
 */

#pragma once

#include "he/core/addable_vector.h"
#include "protocols/common/share.h"
#include "numeric/gfp.h"

template<class U>
class ShareVector : public AddableVector<U>
{
public:
    ShareVector operator+(const ShareVector& other) const
    {
        assert(this->size() == other.size());
        ShareVector res;
        for (size_t i = 0; i < other.size(); i++)
            res.push_back((*this)[i] + other[i]);
        return res;
    }

    ShareVector operator-(const ShareVector& other) const
    {
        assert(this->size() == other.size());
        ShareVector res;
        for (size_t i = 0; i < other.size(); i++)
            res.push_back((*this)[i] - other[i]);
        return res;
    }

    template<class T>
    ShareVector operator*(const AddableVector<T>& other) const
    {
        assert(this->size() == other.size());
        ShareVector res;
        for (size_t i = 0; i < other.size(); i++)
            res.push_back((*this)[i] * other[i]);
        return res;
    }

    template<class T>
    ShareVector operator*(const T& other) const
    {
        ShareVector res;
        for (size_t i = 0; i < this->size(); i++)
            res.push_back((*this)[i] * other);
        return res;
    }

    void fft(const FFT_Data& fftd);
};
