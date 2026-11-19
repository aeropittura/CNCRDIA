/*
 * protocols/replicated/malrep_ring_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"
#include "protocols/replicated/malicious_rep_prep.h"

/**
 * Generate random triples with malicious security modulo a power two,
 * either via larger modulo or shuffling
 */
template<class T>
class MalRepRingPrep : public virtual BufferPrep<T>
{
    DataPositions dummy_position;
    MaliciousRepPrep<typename T::PrepType> prep;

public:
    MalRepRingPrep(SubProcessor<T>* proc, DataPositions& usage);

    void set_protocol(typename T::Protocol&)
    {
    }

    void buffer_triples();
    void simple_buffer_triples();
    void shuffle_buffer_triples();

    void buffer_squares();

    void buffer_inputs(int player);
};

/**
 * Generate random bits from squares modulo a power of two
 */
template<class T>
class RingOnlyBitsFromSquaresPrep : public virtual BufferPrep<T>
{
    typedef typename T::SquareToBitShare BitShare;
    DataPositions dummy_position;
    typename BitShare::SquarePrep prep;
    SubProcessor<BitShare>* bit_processor;
    typename BitShare::MacCheck* bit_opener;

public:
    RingOnlyBitsFromSquaresPrep(SubProcessor<T>* proc, DataPositions& usage);
    ~RingOnlyBitsFromSquaresPrep();

    void buffer_bits();
};

template<class T>
class SimplerMalRepRingPrep : public virtual MalRepRingPrep<T>,
        public virtual RingOnlyBitsFromSquaresPrep<T>
{
public:
    SimplerMalRepRingPrep(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_triples()
    {
        MalRepRingPrep<T>::buffer_triples();
    }

    void buffer_squares()
    {
        MalRepRingPrep<T>::buffer_squares();
    }

    void buffer_bits()
    {
        RingOnlyBitsFromSquaresPrep<T>::buffer_bits();
    }

    void get_dabit_no_count(T& a, typename T::BitType& b)
    {
        this->get_one_no_count(DATA_BIT, a);
        b = a & 1;
    }
};

template<class T>
class MalRepRingPrepWithBits: public virtual MaliciousRingPrep<T>,
        public virtual SimplerMalRepRingPrep<T>
{
public:
    static bool dabits_from_bits()
    {
        return true;
    }

    MalRepRingPrepWithBits(SubProcessor<T>* proc, DataPositions& usage);

    void set_protocol(typename T::Protocol& protocol)
    {
        MaliciousRingPrep<T>::set_protocol(protocol);
    }

    void buffer_squares()
    {
        MalRepRingPrep<T>::buffer_squares();
    }

    void buffer_bits()
    {
        RingOnlyBitsFromSquaresPrep<T>::buffer_bits();
    };
};
