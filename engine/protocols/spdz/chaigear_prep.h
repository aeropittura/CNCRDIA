/*
 * protocols/spdz/chaigear_prep.h
 *
 */

#pragma once

#include "he/offline/simple_generator.h"

/**
 * HighGear/ChaiGear preprocessing
 */
template<class T>
class ChaiGearPrep : public MaliciousRingPrep<T>
{
    template<class U>
    friend typename U::MacKeyType get_mac_key(Player& P, bool read_only);

    typedef typename T::MacKeyType MacKeyType;
    typedef typename T::clear::FD FD;
    typedef SimpleGenerator<SummingEncCommit, FD> Generator;

    static MultiplicativeMachineParameters* machine;
    static Lock lock;

    // setting this non-randomly is a security violation
    static MacKeyType maybe_mac_key;

    Generator* generator;
    SquareProducer<FD>* square_producer;
    InputProducer<FD>* input_producer;

    Generator& get_generator();

    template<int>
    void buffer_bits(true_type);
    template<int>
    void buffer_bits(false_type);

public:
    typedef T ShareType;

    static const bool homomorphic = true;

    static void basic_setup(Player& P, bool read_only = false);
    static void key_setup(Player& P, bool read_only = false);
    static void teardown();

    static MacKeyType get_mac_key(Player& P, bool read_only = false);
    static void adjust_mac_key(Player& P);

    static string get_full_secrets_filename(const Player& P);

    ChaiGearPrep(SubProcessor<T>* proc, DataPositions& usage) :
            BufferPrep<T>(usage), BitPrep<T>(proc, usage),
            RingPrep<T>(proc, usage),
            MaliciousDabitOnlyPrep<T>(proc, usage),
            MaliciousRingPrep<T>(proc, usage), generator(0), square_producer(0),
            input_producer(0)
    {
    }
    ~ChaiGearPrep();

    void buffer_triples();
    void buffer_squares();
    void buffer_bits();
    void buffer_inputs(int player);
};
