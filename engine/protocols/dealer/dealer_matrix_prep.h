/*
 * protocols/dealer/dealer_matrix_prep.h
 *
 */

#pragma once

#include "protocols/common/share_matrix.h"

template<class T>
class DealerMatrixPrep : public BufferPrep<ShareMatrix<T>>
{
    typedef BufferPrep<ShareMatrix<T>> Super;
    typedef typename T::LivePrep LivePrep;

    int n_rows, n_inner, n_cols;

    LivePrep* prep;

public:
    DealerMatrixPrep(int n_rows, int n_inner, int n_cols,
            typename T::LivePrep&, DataPositions& usage);

    void set_protocol(typename ShareMatrix<T>::Protocol&)
    {
    }

    int minimum_batch()
    {
        return -1;
    }

    void buffer_triples();
};
