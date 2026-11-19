/*
 * protocols/common/dummy_matrix_prep.h
 *
 */

#pragma once

#include "vm/data_files.h"
#include "protocols/common/share_matrix.h"

class NoMatrixPrep : public exception
{
};

template<class T>
class DummyMatrixPrep : public Preprocessing<ShareMatrix<T>>
{
public:
    DummyMatrixPrep(int, int, int, Preprocessing<T>&, DataPositions& usage) :
            Preprocessing<ShareMatrix<T>>(usage)
    {
        throw NoMatrixPrep();
    }
};
