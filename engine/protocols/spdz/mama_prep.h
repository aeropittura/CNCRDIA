/*
 * protocols/spdz/mama_prep.h
 *
 */

#pragma once

#include "protocols/spdz/mascot_prep.h"

/**
 * MASCOT triple generation with multiple MACs
 */
template<class T>
class MamaPrep : public MascotInputPrep<T>, public MaliciousRingPrep<T>
{
public:
    static void basic_setup(Player&) {};
    static void teardown() {};

    MamaPrep(SubProcessor<T>* proc, DataPositions& usage);

    void buffer_triples();
};
