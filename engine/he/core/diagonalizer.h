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
    const FftData& FTD;

    size_t n_rows, n_cols;

public:
    typedef AddableVector<ValueMatrix<Gfpvar>> MatrixVector;

    vector<Ciphertext> ciphertexts;

    Diagonalizer(const MatrixVector& matrices,
            const FftData& FTD, const FHE_PK& pk);

    Plaintext_<FftData> get_plaintext(const MatrixVector& matrices,
            int left_column, int right_column);

    MatrixVector decrypt(const vector<Ciphertext>&, int n_matrices, FHE_SK& sk);

    MatrixVector dediag(const vector<Plaintext_<FftData>>& plaintexts,
            int n_matrices);
};
