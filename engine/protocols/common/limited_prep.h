/*
 * protocols/common/limited_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"

template<class T>
class LimitedPrep : public BufferPrep<T>
{
    DataPositions usage;

    void buffer_triples() { throw ran_out(); }
    void buffer_squares() { throw ran_out(); }
    void buffer_inverses() { throw ran_out(); }
    void buffer_bits() { throw ran_out(); }

public:
    LimitedPrep();
    ~LimitedPrep();

    void set_protocol(typename T::Protocol& protocol);
};
