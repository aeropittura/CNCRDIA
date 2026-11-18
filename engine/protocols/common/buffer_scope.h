/*
 * protocols/common/buffer_scope.h
 *
 */

#pragma once

template<class T> class BufferPrep;
template<class T> class Preprocessing;

#include "vm/online_options.h"

template<class T>
class BufferScope
{
    T& prep;
    int bak;

public:
    BufferScope(T& prep, int buffer_size) :
            prep(prep)
    {
        if (OnlineOptions::singleton.has_option("debug_batch_size"))
            cerr << "scope batch_size=" << buffer_size << endl;
        bak = this->prep.buffer_size;
        this->prep.buffer_size = buffer_size;
    }

    ~BufferScope()
    {
        prep.buffer_size = bak;
    }
};
