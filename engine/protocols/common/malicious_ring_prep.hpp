/*
 * protocols/common/malicious_ring_prep.hpp
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"

#include "protocols/common/dabit_sacrifice.hpp"
#include "protocols/spdz2k/spdz2k_prep.hpp"
#include "protocols/common/shuffle_sacrifice.hpp"

template<class T>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues)
{
    buffer_dabits<0>(queues, T::clear::characteristic_two,
            T::clear::prime_field);
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues*, true_type, false_type)
{
    throw runtime_error("only implemented for integer-like domains");
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues, false_type, false_type)
{
    assert(this->proc != 0);
    vector<dabit<T>> check_dabits;
    this->buffer_dabits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(this->buffer_size), queues);
    dabit_sacrifice.sacrifice_and_check_bits(this->dabits, check_dabits,
            *this->proc, queues);
}

template<class T>
template<int>
void MaliciousDabitOnlyPrep<T>::buffer_dabits(ThreadQueues* queues, false_type, true_type)
{
    if (T::clear::length() >= 60)
        buffer_dabits<0>(queues, false_type(), false_type());
    else
    {
        assert(this->proc != 0);
        vector<dabit<T>> check_dabits;
        DabitShuffleSacrifice<T> shuffle_sacrifice;
        this->buffer_dabits_without_check(check_dabits,
                shuffle_sacrifice.minimum_n_inputs(), queues);
        shuffle_sacrifice.dabit_sacrifice(this->dabits, check_dabits, *this->proc,
                queues);
    }
}
