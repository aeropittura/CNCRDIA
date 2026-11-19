/*
 * protocols/common/shuffle_sacrifice.h
 *
 */

#pragma once

#include <vector>
#include <array>
using namespace std;

#include "support/fixed_vector.h"
#include "protocols/common/edabit.h"
#include "protocols/common/dabit.h"

class Player;

template<class T> class LimitedPrep;

/**
 * Base class for shuffle sacrificing
 */
class ShuffleSacrifice
{
protected:
    const int B;

public:
    const int C;

    ShuffleSacrifice();
    ShuffleSacrifice(int B, int C = 3);

    int minimum_n_inputs(int n_outputs = 1)
    {
        return max(n_outputs, minimum_n_outputs()) * B + C;
    }
    int minimum_n_inputs_with_combining(int n_outputs = 1)
    {
        return minimum_n_inputs(B * max(n_outputs, minimum_n_outputs()));
    }
    int minimum_n_outputs()
    {
        if (B == 3)
            return 1 << 20;
        else if (B == 4)
            return 10368;
        else if (B == 5)
            return 1024;
        else
            throw runtime_error("not supported: B = " + to_string(B));
    }

    template<class U>
    void shuffle(vector<U>& items, Player& P);
};

template<class T>
class TripleShuffleSacrifice : public ShuffleSacrifice
{
public:
    static int batch_size();

    TripleShuffleSacrifice();
    TripleShuffleSacrifice(int B);
    TripleShuffleSacrifice(int B, int C);

    void triple_sacrifice(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& check_triples, Player& P,
            typename T::MacCheck& MC, ThreadQueues* queues = 0);
    void triple_sacrifice(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& check_triples, Player& P,
            typename T::MacCheck& MC, int begin, int end);

    void triple_combine(vector<array<T, 3>>& triples,
            vector<array<T, 3>>& to_combine, Player& P,
            typename T::MacCheck& MC);
};

template<class T>
class DabitShuffleSacrifice : public ShuffleSacrifice
{
public:
    void dabit_sacrifice(vector<Dabit<T>>& dabits,
            vector<Dabit<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);
};

template<class T>
class EdabitShuffleSacrifice : public ShuffleSacrifice
{
  typedef typename T::BitType::part_type BT;

    size_t n_bits;

public:
    EdabitShuffleSacrifice(int n_bits);

    void edabit_sacrifice(vector<Edabit<T>>& output, vector<T>& sums,
            vector<vector<typename T::BitType::part_type>>& bits,
            SubProcessor<T>& proc, bool strict = false, int player = -1,
            ThreadQueues* = 0);

    void edabit_sacrifice_buckets(vector<Edabit<T>>& to_check,
            bool strict, int player, SubProcessor<T>& proc, int begin, int end,
            const void* supply = 0);

    void edabit_sacrifice_buckets(vector<Edabit<T>>& to_check,
            bool strict, int player, SubProcessor<T>& proc, int begin, int end,
            LimitedPrep<BT>& personal_prep, const void* supply = 0);
};
