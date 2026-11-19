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

    void buffer_triples() { throw RanOut(); }
    void buffer_squares() { throw RanOut(); }
    void buffer_inverses() { throw RanOut(); }
    void buffer_bits() { throw RanOut(); }

public:
    LimitedPrep();
    ~LimitedPrep();

    void set_protocol(typename T::Protocol& protocol);
};
