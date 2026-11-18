/*
 * vm/no_file_prep.h
 *
 */

#pragma once

#include "vm/data_files.h"

template<class T>
class NoFilePrep : public Preprocessing<T>
{
public:
    NoFilePrep(int, int, const string&, DataPositions& usage, int = -1) :
            Preprocessing<T>(usage)
    {
        throw runtime_error("preprocessing from file disabled");
    }
};
