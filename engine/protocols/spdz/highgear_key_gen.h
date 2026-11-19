/*
 * protocols/spdz/highgear_key_gen.h
 *
 */

#pragma once

#include "protocols/spdz/lowgear_key_gen.h"

#include <deque>
using namespace std;

template<class T, class U>
class KeyGeneratorBitFactory
{
    U& keygen;
    deque<T>& buffer;

public:
    KeyGeneratorBitFactory(U& keygen, deque<T>& buffer) :
            keygen(keygen), buffer(buffer)
    {
    }

    T get_bit()
    {
        if (buffer.empty())
            keygen.buffer_mabits();
        auto res = buffer.front();
        buffer.pop_front();
        return res;
    }
};

/**
 * Somewhat homomorphic encryption key generation using MASCOT
 */
template<int L, int M>
class HighGearKeyGenerator
{
public:
    typedef KeyGeneratorProtocol<1, -1> Proto0;
    typedef KeyGeneratorProtocol<2, -1> Proto1;

    typedef typename Proto0::ShareType ShareType0;
    typedef typename Proto1::ShareType ShareType1;

    typedef typename ShareType0::OpenType OpenType0;
    typedef typename ShareType1::OpenType OpenType1;

    typedef ShareVector<ShareType0> VectorType0;
    typedef ShareVector<ShareType1> VectorType1;

    typedef typename ShareType0::BitType BT;

    Player& P;
    const FheParameters& params;

    Proto0 proto0;
    Proto1 proto1;

    deque<ShareType0> bits0;
    deque<ShareType1> bits1;

    HighGearKeyGenerator(Player& P, const FheParameters& params);

    void buffer_mabits();

    template<class FD>
    void run(PartSetup<FD>& setup, MachineBase& machine);
};
