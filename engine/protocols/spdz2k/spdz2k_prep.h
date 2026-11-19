/*
 * protocols/spdz2k/spdz2k_prep.h
 *
 */

#pragma once

#include "protocols/spdz/mascot_prep.h"
#include "protocols/common/ring_only_prep.h"

template<int, int> class Spdz2kShare;

template<class T, class U>
void bits_from_square_in_ring(vector<T>& bits, int buffer_size, U* bit_prep);

/**
 * Spdz2k preprocessing
 */
template<class T>
class Spdz2kPrep : public virtual MaliciousRingPrep<T>,
        public virtual MascotTriplePrep<T>,
        public virtual RingOnlyPrep<T>
{
    typedef typename T::BitPrepType BitShare;
    DataPositions bit_position;
    MascotTriplePrep<BitShare>* bit_prep;
    SubProcessor<BitShare>* bit_processor;
    typename BitShare::MacCheck* bit_opener;
    DabitSacrifice<T> dabit_sacrifice;

public:
    Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~Spdz2kPrep();

    void set_protocol(typename T::Protocol& protocol);

    void buffer_inverses() { throw DivisionByZero(); }
    void buffer_bits();
    void buffer_dabits(ThreadQueues* queues);

#ifdef SPDZ2K_BIT
    void get_dabit(T& a, GC::TinySecret<T::s>& b);
#endif
};
