/*
 * protocols/spdz2k/spdz2k_prep.hpp
 *
 */

#pragma once

#include "protocols/spdz2k/spdz2k_prep.h"
#include "boolean/bit_adder.h"

#include "protocols/common/dabit_sacrifice.hpp"
#include "protocols/common/ring_only_prep.hpp"

template<class T>
Spdz2kPrep<T>::Spdz2kPrep(SubProcessor<T>* proc, DataPositions& usage) :
        BufferPrep<T>(usage),
        BitPrep<T>(proc, usage), RingPrep<T>(proc, usage),
        MaliciousDabitOnlyPrep<T>(proc, usage),
        MaliciousRingPrep<T>(proc, usage),
        MascotTriplePrep<T>(proc, usage),
        RingOnlyPrep<T>(proc, usage)
{
    this->params.amplify = false;
    bit_opener = 0;
    bit_processor = 0;
    bit_prep = 0;
}

template<class T>
Spdz2kPrep<T>::~Spdz2kPrep()
{
    if (bit_prep != 0)
    {
        delete bit_processor;
        delete bit_prep;
        delete bit_opener;
    }
}

template<class T>
void Spdz2kPrep<T>::set_protocol(typename T::Protocol& protocol)
{
    OtPrep<T>::set_protocol(protocol);
    assert(this->proc != 0);
    auto& proc = this->proc;
    bit_opener = new typename BitShare::MacCheck(proc->MC.get_alphai());
    // just dummies
    bit_position = DataPositions(proc->P.num_players());
    bit_prep = new MascotTriplePrep<BitShare>(bit_processor, bit_position);
    bit_prep->params.amplify = false;
    bit_processor = new SubProcessor<BitShare>(*bit_opener, *bit_prep, proc->P);
    bit_opener->set_prep(*bit_prep);
}

template<class T>
void MaliciousRingPrep<T>::buffer_bits()
{
    CODE_LOCATION
    assert(this->proc != 0);
    RingPrep<T>::buffer_bits_without_check();
    assert(this->protocol != 0);
    auto& protocol = *this->protocol;
    protocol.initialize_dotprod();
    auto one = T::constant(1, protocol.P.my_num(), this->proc->MC.get_alphai());
    GlobalPrng G(protocol.P);
    for (auto& bit : this->bits)
        // one of the two is not a zero divisor, so if the product is zero, one of them is too
        protocol.prepare_dotprod(one - bit, bit * G.get<typename T::OpenType>());
    protocol.next_dotprod();
    protocol.exchange();
    this->proc->MC.check_for(0, {protocol.finalize_dotprod(this->bits.size())},
            protocol.P);
}

template<class T>
void Spdz2kPrep<T>::buffer_bits()
{
#ifdef SPDZ2K_SIMPLE_BITS
    MaliciousRingPrep<T>::buffer_bits();
#else
    bits_from_square_in_ring(this->bits, this->buffer_size, bit_prep);
#endif
}

template<class T, class U>
void bits_from_square_in_ring(vector<T>& bits, int buffer_size, U* bit_prep)
{
    CODE_LOCATION
    buffer_size = BaseMachine::batch_size<T>(DATA_BIT, buffer_size);
    typedef typename U::ShareType BitShare;
    typedef typename BitShare::OpenType OpenType;
    assert(bit_prep != 0);
    auto bit_processor = bit_prep->get_processor();
    assert(bit_processor != 0);
    auto bit_opener = &bit_processor->MC;
    vector<BitShare> squares, random_shares;
    auto one = BitShare::constant(1, bit_processor->P.my_num(), bit_opener->get_alphai());
    BufferScope scope(*bit_prep, buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        BitShare a, a2;
        bit_prep->get_two(DATA_SQUARE, a, a2);
        squares.push_back((a2 + a) * 4 + one);
        random_shares.push_back(a * 2 + one);
    }
    vector<typename BitShare::OpenType> opened;
    bit_opener->POpen(opened, squares, bit_processor->P);
    for (int i = 0; i < buffer_size; i++)
    {
        auto& root = opened[i];
        if (root.get_bit(0) != 1)
            throw runtime_error("square not odd");
        BitShare tmp = random_shares[i] * OpenType(root.square_root().invert()) + one;
        bits.push_back(tmp >> 1);
    }
    bit_opener->Check(bit_processor->P);
}

#ifdef SPDZ2K_BIT
template<class T>
void Spdz2kPrep<T>::get_dabit(T& a, GC::TinySecret<T::s>& b)
{
    this->get_one(DATA_BIT, a);
    b.resize_registers(1);
    b.get_reg(0) = Spdz2kShare<1, T::s>(a);
}
#endif

template<class T>
void Spdz2kPrep<T>::buffer_dabits(ThreadQueues* queues)
{
    assert(this->proc != 0);
    vector<Dabit<T>> check_dabits;
    int buffer_size = BaseMachine::batch_size<T>(DATA_DABIT, this->buffer_size);
    this->buffer_dabits_from_bits_without_check(check_dabits,
            dabit_sacrifice.minimum_n_inputs(buffer_size), queues);
    dabit_sacrifice.sacrifice_without_bit_check(this->dabits, check_dabits,
            *this->proc, queues);
}

template<class T>
void MascotPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
    this->buffer_edabits_from_personal(strict, n_bits, queues);
}

template<class T>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool strict, int n_bits,
        ThreadQueues* queues)
{
    buffer_edabits_from_personal<0>(strict, n_bits, queues,
            T::clear::characteristic_two);
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool, int,
        ThreadQueues*, true_type)
{
    throw runtime_error("only implemented for integer-like domains");
}

template<class T>
template<int>
void MaliciousRingPrep<T>::buffer_edabits_from_personal(bool strict, int n_bits,
        ThreadQueues* queues, false_type)
{
    CODE_LOCATION
    assert(this->proc != 0);
    typedef typename T::BitType::part_type BitType;
    vector<vector<BitType>> bits;
    vector<T> sums;

    bool verbose = OnlineOptions::singleton.has_option("verbose_eda");
    Timer timer;
    if (verbose)
    {
        cerr << "Generate edaBits of length " << n_bits << " to sacrifice" << endl;
        timer.start();
    }

    auto &party = GC::ShareThread<typename T::BitType>::s();
    SubProcessor<BitType> bit_processor(party.MC->get_part_opener(),
            this->proc->bit_prep, this->proc->P);
    int n_relevant = this->proc->protocol.get_n_relevant_players();
    vector<vector<vector<BitType>>> player_bits(n_bits);
    for (int i = 0; i < n_relevant; i++)
    {
        vector<T> tmp;
        vector<vector<BitType>> tmp_bits;
        this->buffer_personal_edabits(n_bits, tmp, tmp_bits, bit_processor,
                i, strict, queues);
        assert(not tmp.empty());
        sums.resize(tmp.size());
        for (size_t j = 0; j < tmp.size(); j++)
            sums[j] += tmp[j];
        for (int j = 0; j < n_bits; j++)
            player_bits[j].push_back(tmp_bits[j]);
    }
    RunningTimer add_timer;
    BitAdder().add(bits, player_bits, bit_processor, BitType::default_length,
            queues);
    player_bits.clear();

    if (verbose)
    {
        cerr << "Adding edaBits took " << add_timer.elapsed() << " seconds"
                << endl;
        cerr << "Done with generating edaBits after " << timer.elapsed()
                << " seconds" << endl;
    }

    RunningTimer finalize_timer;
    vector<Edabit<T>> checked;
    for (size_t i = 0; i < sums.size(); i++)
    {
        checked.push_back({sums[i], {}});
        int i1 = i / BitType::default_length;
        int i2 = i % BitType::default_length;
        checked.back().second.reserve(bits.at(i1).size());
        for (auto& x : bits.at(i1))
            checked.back().second.push_back(x.get_bit(i2));
    }
    sums.clear();
    sums.shrink_to_fit();
    bits.clear();
    bits.shrink_to_fit();
    if (strict)
        this->template sanitize<0>(checked, n_bits, -1, queues);
    auto& output = this->edabits[{strict, n_bits}];
    for (auto& x : checked)
    {
        if (output.empty() or output.back().full())
            output.push_back(x);
        else
            output.back().push_back(x);
    }

    if (verbose)
        cerr << "Finalizing took " << finalize_timer.elapsed() << " seconds" << endl;
}

template<class T>
void MaliciousRingPrep<T>::buffer_edabits(bool strict, int n_bits,
        ThreadQueues* queues)
{
    CODE_LOCATION
    RunningTimer timer;
#ifndef NONPERSONAL_EDA
    this->buffer_edabits_from_personal(strict, n_bits, queues);
#else
    assert(this->proc != 0);
    ShuffleSacrifice<T> shuffle_sacrifice;
    typedef typename T::BitType::part_type BitType;
    vector<vector<BitType>> bits;
    vector<T> sums;
    this->buffer_edabits_without_check(n_bits, sums, bits,
            shuffle_sacrifice.minimum_n_inputs(), queues);
    vector<Edabit<T>>& checked = this->edabits[{strict, n_bits}];
    shuffle_sacrifice.edabit_sacrifice(checked, sums, bits,
            n_bits, *this->proc, strict, -1, queues);
    if (strict)
        this->sanitize(checked, n_bits, -1, queues);
#endif
#ifdef VERBOSE_EDA
    cerr << "Total edaBit generation took " << timer.elapsed() << " seconds" << endl;
#endif
}
