/*
 * protocols/common/limited_prep.hpp
 *
 */

#pragma once

#include "protocols/common/limited_prep.h"

template<class T>
LimitedPrep<T>::LimitedPrep() :
        BufferPrep<T>(usage)
{
}

template<class T>
LimitedPrep<T>::~LimitedPrep()
{
#ifdef VERBOSE
    if (not this->triples.empty())
        cerr << "Triples left" << endl;
#endif
}

template<class T>
void LimitedPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    usage.set_num_players(protocol.P.num_players());
}
