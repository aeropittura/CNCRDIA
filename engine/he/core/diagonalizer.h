/*
 * he/core/diagonalizer.h
 *
 */

#pragma once

#include "numeric/gfpvar.h"
#include "he/core/ciphertext.h"
#include "protocols/common/share_matrix.h"

class Diagonalizer
{
    const FFT_Data& FTD;

    size_t n_rows, n_cols;

public:
    typedef AddableVector<ValueMatrix<gfpvar>> MatrixVector;

    vector<Ciphertext> ciphertexts;

    Diagonalizer(const MatrixVector& matrices,
            const FFT_Data& FTD, const FHE_PK& pk);

    Plaintext_<FFT_Data> get_plaintext(const MatrixVector& matrices,
            int left_col, int right_col);

    MatrixVector decrypt(const vector<Ciphertext>&, int n_matrices, FHE_SK& sk);

    MatrixVector dediag(const vector<Plaintext_<FFT_Data>>& plaintexts,
            int n_matrices);
};
