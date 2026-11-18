/*
 * NoFilePrep.h
 *
 */

#ifndef PROCESSOR_NOFILEPREP_H_
#define PROCESSOR_NOFILEPREP_H_

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

#endif /* PROCESSOR_NOFILEPREP_H_ */
