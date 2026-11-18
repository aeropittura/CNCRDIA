/*
 * protocols/common/dummy_matrix_prep.h
 *
 */

#pragma once

#include "vm/data_files.h"
#include "protocols/common/share_matrix.h"

class no_matrix_prep : public exception
{
};

template<class T>
class DummyMatrixPrep : public Preprocessing<ShareMatrix<T>>
{
public:
    DummyMatrixPrep(int, int, int, Preprocessing<T>&, DataPositions& usage) :
            Preprocessing<ShareMatrix<T>>(usage)
    {
        throw no_matrix_prep();
    }
};
