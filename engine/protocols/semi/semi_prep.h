/*
 * protocols/semi/semi_prep.h
 *
 */

#pragma once

#include "protocols/spdz/mascot_prep.h"

template<class T> class HemiPrep;

/**
 * Semi-honest triple generation based on oblivious transfer
 */
template<class T>
class SemiPrep : public virtual OtPrep<T>, public virtual SemiHonestRingPrep<T>
{
    friend class HemiPrep<T>;

public:
    SemiPrep(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_triples();

    void buffer_dabits(ThreadQueues* queues);

    void get_one_no_count(Dtype dtype, T& a);

    static bool bits_from_dabits();
};
