/*
 * protocols/replicated/rep3_shuffler.h
 *
 */

#pragma once

#include "protocols/common/secure_shuffle.h"

template<class T>
class Rep3Shuffler : public SecureShuffleBase<T>
{
public:
    typedef array<CheckVector<int>, 2> ShuffleType;
    typedef ShuffleStore<ShuffleType> StoreType;

private:
    SubProcessor<T>& proc;

    CheckVector<CheckVector<T>> to_shuffle;
    CheckVector<typename T::clear> to_share;

public:
    map<long, long> statistics;

    Rep3Shuffler(SubProcessor<T>& proc);

    void generate(int n_shuffle, ShuffleType& shuffle);

    void apply_multiple(StackedVector<T>& a, vector<ShuffleTuple<ShuffleType>> &shuffles,
            bool multithread = true);

    void shuffle_job(StackedVector<T>& a, const vector<ShuffleTuple<ShuffleType>>& shuffles,
            long begin, long end, Player& P, ThreadQueues* queues);
};
