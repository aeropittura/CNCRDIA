/*
 * protocols/spdzwise/spdzwise_rep3_shuffler.h
 *
 */

#pragma once

#include "protocols/replicated/rep3_shuffler.h"
#include "protocols/common/protocol_set.h"

template<class T>
class SpdzWiseRep3Shuffler : public SecureShuffleBase<T>
{
    SubProcessor<T>& proc;

    ProtocolSet<typename T::part_type::Honest> internal_set;
    Rep3Shuffler<typename T::part_type::Honest> internal;

public:
    typedef typename Rep3Shuffler<T>::StoreType StoreType;
    typedef typename Rep3Shuffler<T>::ShuffleType ShuffleType;

    map<long, long> statistics;

    SpdzWiseRep3Shuffler(SubProcessor<T>& proc);

    void generate(int n_shuffle, ShuffleType& shuffle);

    void apply_multiple(StackedVector<T>& a, vector<ShuffleTuple<ShuffleType>>& shuffles);
};
