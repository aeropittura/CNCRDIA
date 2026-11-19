/*
 * protocols/common/dabit_sacrifice.h
 *
 */

#pragma once

#include "vm/base_machine.h"

template<class T>
class DabitSacrifice
{
    const int S;

    size_t n_masks, n_produced;

public:
    DabitSacrifice();
    ~DabitSacrifice();

    int minimum_n_inputs(int n_outputs = 0)
    {
        if (T::clear::N_BITS < 0)
            // sacrifice uses S^2 random bits
            n_outputs = BaseMachine::batch_size<T>(DATA_DABIT,
                    n_outputs, max(n_outputs, 10 * S * S));
        else
            n_outputs = BaseMachine::batch_size<T>(DATA_DABIT, n_outputs);
        assert(n_outputs > 0);
        return n_outputs + S;
    }

    void sacrifice_without_bit_check(vector<Dabit<T>>& dabits,
            vector<Dabit<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);

    void sacrifice_and_check_bits(vector<Dabit<T>>& dabits,
            vector<Dabit<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);
};
