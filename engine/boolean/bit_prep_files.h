/*
 * boolean/bit_prep_files.h
 *
 */

#pragma once

#include "boolean/shiftable_triple_buffer.h"
#include "vm/data_files.h"

namespace GC
{

template<class T>
class BitPrepFiles : public ShiftableTripleBuffer<T>, public SubDataFiles<T>
{
public:
    BitPrepFiles(const Names& N, const string& prep_data_directory,
            DataPositions& usage, int thread_num = -1) :
            SubDataFiles<T>(N, prep_data_directory, usage, thread_num)
    {
    }

    array<T, 3> get_triple_no_count(int n_bits)
    {
        return ShiftableTripleBuffer<T>::get_triple_no_count(n_bits);
    }

    void get(Dtype type, T* data)
    {
        SubDataFiles<T>::get(type, data);
    }
};

}
