/*
 * protocols/semi/hemi_matrix_prep.h
 *
 */

#pragma once

#include "protocols/common/share_matrix.h"
#include "protocols/common/replicated_prep.h"

template<class T> class HemiPrep;

/**
 * Semi-honest matrix triple generation using semi-homomorphic encryption
 */
template<class T>
class HemiMatrixPrep : public BufferPrep<ShareMatrix<T>>
{
    typedef BufferPrep<ShareMatrix<T>> super;
    typedef typename T::LivePrep LivePrep;

    int n_rows, n_inner, n_cols;
    bool swapped;

    LivePrep* prep;

    HemiMatrixPrep(const HemiMatrixPrep&) = delete;

public:
    static const bool homomorphic = true;

    HemiMatrixPrep(int n_rows, int n_inner, int n_cols, LivePrep& prep,
            DataPositions& usage) :
            super(usage), n_rows(n_rows), n_inner(n_inner),
            n_cols(n_cols), prep(&prep)
    {
        swapped = n_rows > n_cols;
        if (swapped)
            std::swap(this->n_rows, this->n_cols);
        assert(this->n_cols >= this->n_rows);
        assert(prep.proc);
        this->P = &prep.proc->P;
    }

    int minimum_batch();

    void set_protocol(typename ShareMatrix<T>::Protocol&)
    {
    }

    void buffer_triples();
};
