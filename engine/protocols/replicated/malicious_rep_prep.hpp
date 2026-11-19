/*
 * protocols/replicated/malicious_rep_prep.hpp
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep_prep.h"
#include "protocols/common/buffer_scope.h"
#include "support/subroutines.h"
#include "vm/online_options.h"

#include "protocols/common/mac_key.hpp"

template<class T>
MaliciousBitOnlyRepPrep<T>::MaliciousBitOnlyRepPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage),
        honest_prep(0, honest_usage), honest_processor(0)
{
    this->proc = proc;
}

template<class T>
MaliciousRepPrep<T>::MaliciousRepPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage), MaliciousBitOnlyRepPrep<T>(proc, usage)
{
}

template<class T>
MaliciousRepPrep<T>::MaliciousRepPrep(DataPositions& usage, int) :
        MaliciousRepPrep<T>(0, usage)
{
}

template<class T>
template<class V>
MaliciousRepPrep<T>::MaliciousRepPrep(DataPositions& usage,
        GC::ShareThread<V>&, int) :
        MaliciousRepPrep<T>(0, usage)
{
}

template<class T>
MaliciousRepPrepWithBits<T>::MaliciousRepPrepWithBits(SubProcessor<T>* proc,
        DataPositions& usage) :
        BufferPrep<T>(usage), MaliciousRepPrep<T>(proc, usage),
        BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
        MaliciousDabitOnlyPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage)
{
}

template<class U>
MaliciousBitOnlyRepPrep<U>::~MaliciousBitOnlyRepPrep()
{
    if (honest_processor)
        delete honest_processor;
}

template<class T>
void MaliciousBitOnlyRepPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    initialize_honest(protocol.P);
}

template<class T>
void MaliciousBitOnlyRepPrep<T>::initialize_honest(Player& P)
{
    if (not honest_processor)
        honest_processor = new SubProcessor<typename T::Honest>(honest_opener,
                honest_prep, P);
}

template<class T>
void MaliciousRepPrep<T>::buffer_triples()
{
    CODE_LOCATION
    check_field_size<typename T::OpenType>();
    auto& triples = this->triples;
    auto buffer_size = BaseMachine::batch_size<T>(DATA_TRIPLE,
            this->buffer_size);
    if (OnlineOptions::singleton.has_option("verbose_triples"))
        fprintf(stderr, "creating %d triples (%d)\n", buffer_size,
                this->buffer_size);
    auto& honest_processor = this->honest_processor;
    assert(honest_processor != 0);
    Player& P = honest_processor->P;
    vector<array<T, 5>> check_triples;
    check_triples.reserve(buffer_size);
    auto& honest_protocol = honest_processor->protocol;
    honest_protocol.initialize_mult();
    BufferScope scope(honest_protocol, 3 * buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        check_triples.push_back({});
        auto& tuple = check_triples.back();
        for (int j = 0; j < 3; j++)
            tuple[j] = honest_protocol.get_random();
        honest_protocol.prepare_mul(tuple[0], tuple[2]);
        honest_protocol.prepare_mul(tuple[1], tuple[2]);
    }
    honest_protocol.exchange();
    for (int i = 0; i < buffer_size; i++)
        for (int j = 3; j < 5; j++)
            check_triples[i][j] = honest_protocol.finalize_mul();
    sacrifice<T, typename T::random_type>(check_triples, P);
    for (auto& tuple : check_triples)
        triples.push_back({{tuple[0], tuple[2], tuple[3]}});
}

template<class T, class U>
void sacrifice(const vector<array<T, 5>>& check_triples, Player& P)
{
    CODE_LOCATION
    check_field_size<U>();
    vector<T> masked, checks;
    vector <typename T::OpenType> opened;
    typename T::MacCheck MC;
    int buffer_size = check_triples.size();
    auto t = create_random<U>(P);
    masked.reserve(buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        const T& a = check_triples[i][0];
        const T& a_prime = check_triples[i][1];
        masked.push_back(T::Mul(a, t) - a_prime);
    }
    MC.POpen(opened, masked, P);
    checks.reserve(buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        const T& b = check_triples[i][2];
        const T& c = check_triples[i][3];
        const T& c_prime = check_triples[i][4];
        typename T::OpenType& rho = opened[i];
        checks.push_back(T::Mul(c, t) - c_prime - rho * b);
    }
    MC.check_for(0, checks, P);
    MC.Check(P);
}

template<class T>
void MaliciousRepPrep<T>::buffer_squares()
{
    CODE_LOCATION
    vector<T> masked;
    vector<T> checks;
    vector<typename T::OpenType> opened;
    vector<array<T, 2>> check_squares;
    auto& squares = this->squares;
    auto buffer_size = BaseMachine::batch_size<T>(DATA_SQUARE, this->buffer_size);
    auto& honest_prep = this->honest_prep;
    auto& honest_processor = this->honest_processor;
    auto& MC = this->MC;
    assert(honest_processor);
    Player& P = honest_processor->P;
    squares.clear();
    BufferScope scope(honest_prep, buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        T a, b;
        T f, h;
        honest_prep.get_two(DATA_SQUARE, a, b);
        honest_prep.get_two(DATA_SQUARE, f, h);
        squares.push_back({{a, b}});
        check_squares.push_back({{f, h}});
    }
    auto t = create_random<typename T::clear>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = squares[i][0];
        T& f = check_squares[i][0];
        masked.push_back(a * t - f);
    }
    MC.POpen(opened, masked, P);
    typename T::clear t2 = t * t;
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = squares[i][0];
        T& b = squares[i][1];
        T& f = check_squares[i][0];
        T& h = check_squares[i][1];
        auto& rho = opened[i];
        checks.push_back(t2 * b - h - rho * (t * a + f));
    }
    MC.check_for(0, checks, P);
}

template<class T>
void MaliciousBitOnlyRepPrep<T>::buffer_bits()
{
    CODE_LOCATION
    vector<T> masked;
    vector<T> checks;
    vector<typename T::OpenType> opened;
    vector<array<T, 2>> check_squares;
    auto& bits = this->bits;
    auto buffer_size = BaseMachine::batch_size<T>(DATA_BIT,
            this->buffer_size);
    assert(honest_processor);
    Player& P = honest_processor->P;
    BufferScope scope(honest_prep, buffer_size);
    bits.clear();
    for (int i = 0; i < buffer_size; i++)
    {
        T a, f, h;
        honest_prep.get_one(DATA_BIT, a);
        honest_prep.get_two(DATA_SQUARE, f, h);
        bits.push_back(a);
        check_squares.push_back({{f, h}});
#ifdef DEBUG_BIT_SACRIFICE
        typename T::MacCheck MC;
        if (not (MC.open(a, P).is_zero() or MC.open(a, P).is_one()))
        {
            cout << MC.open(a, P) << endl;
            throw exception();
        }
        assert(MC.open(f, P) * MC.open(f, P) == MC.open(h, P));
#endif
    }
    auto t = create_random<typename T::OpenType>(P);
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = bits[i];
        T& f = check_squares[i][0];
        masked.push_back(t * a - f);
    }
    MC.POpen(opened, masked, P);
    typename T::OpenType t2 = t * t;
    for (int i = 0; i < buffer_size; i++)
    {
        T& a = bits[i];
        T& f = check_squares[i][0];
        T& h = check_squares[i][1];
        auto& rho = opened[i];
        checks.push_back(t2 * a - h - rho * (t * a + f));
    }
    MC.check_for(0, checks, P);
}

template<class T>
void MaliciousRepPrep<T>::buffer_inputs(int player)
{
    auto proc = this->proc;
    assert(proc);
    this->buffer_inputs_as_usual(player, proc);
}
