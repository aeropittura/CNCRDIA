/*
 * ot/ot_multiplier.hpp
 *
 */

#pragma once

#include "ot/config.h"

#include "ot/ot_multiplier.h"
#include "ot/n_party_triple_generator.h"
#include "ot/base_ot.h"

#include "ot/ot_vole.hpp"
#include "ot/row.hpp"
#include "ot/rectangle.hpp"
#include "ot/ot_correlator.hpp"

#include <math.h>

//#define OTCORR_TIMER

template<class T>
OtMultiplier<T>::OtMultiplier(OtTripleGenerator<T>& generator,
        int thread_num) :
        generator(generator), thread_num(thread_num),
        rot_ext(generator.players[thread_num], BOTH,
                !generator.machine.correlation_check),
        ot_correlator(generator.players[thread_num], BOTH, true)
{
    this->thread = 0;
    rot_ext.init(generator.base_receiver_input,
            generator.base_sender_inputs[thread_num],
            generator.base_receiver_outputs[thread_num]);
}

template<class T>
MascotMultiplier<T>::MascotMultiplier(OtTripleGenerator<T>& generator,
        int thread_num) :
        OtMultiplier<T>(generator, thread_num),
		auth_ot_ext(generator.players[thread_num], BOTH, true)
{
    c_output.resize(generator.n_triples_per_loop);
}

template<class T>
TinyMultiplier<T>::TinyMultiplier(OtTripleGenerator<T>& generator,
        int thread_num) :
        OtMultiplier<T>(generator, thread_num),
        mac_vole(T::part_type::MacKeyType::N_BITS,
                generator.players[thread_num],
                BOTH, false)
{
    c_output.resize(generator.n_triples_per_loop);
}

template<class T>
TinierMultiplier<T>::TinierMultiplier(OtTripleGenerator<T>& generator,
        int thread_num) :
        OtMultiplier<T>(generator, thread_num),
        auth_ot_ext(generator.players[thread_num], BOTH, true)
{
    c_output.resize(generator.n_triples_per_loop);
}

template <int K, int S>
Spdz2kMultiplier<K, S>::Spdz2kMultiplier(OtTripleGenerator<Spdz2kShare<K, S>>& generator, int thread_num) :
        OtMultiplier<Spdz2kShare<K, S>>
        (generator, thread_num)
{
#ifdef USE_OPT_VOLE
		mac_vole = new OtVole<Z2<MAC_BITS>>(S, generator.players[thread_num], BOTH, false);
		input_mac_vole = new OtVole<Z2<K + S>>(S, generator.players[thread_num], BOTH, false);
#else
		mac_vole = new OtVoleBase<Z2<MAC_BITS>>(S, generator.players[thread_num], BOTH, false);
		input_mac_vole = new OtVoleBase<Z2<K + S>>(S, generator.players[thread_num], BOTH, false);
#endif
}

template<int K, int S>
Spdz2kMultiplier<K, S>::~Spdz2kMultiplier()
{
    delete mac_vole;
    delete input_mac_vole;
}

template<class T>
OtMultiplier<T>::~OtMultiplier()
{
}

template<class U>
void OtMultiplierBase::init(U& key, Player& P, int other, OtTripleSetup& ot_setup)
{
    VirtualTwoPartyPlayer player(P, other);
    OtExtensionWithMatrix rot_ext(&player);
    int index = key.ot.index_for(other);
    rot_ext.init(ot_setup.base_receiver_inputs,
            ot_setup.base_sender_inputs.at(index),
            ot_setup.base_receiver_outputs.at(index));
    auto& key_bits = key.ot.base_receiver_inputs;
    rot_ext.extend(key_bits.size(), key_bits);
    auto& senderOutput = key.ot.base_sender_inputs.at(index);
    auto& receiverOutput = key.ot.base_receiver_outputs.at(index);
    senderOutput.resize(key_bits.size());
    for (size_t j = 0; j < key_bits.size(); j++)
    {
        for (int i = 0; i < 2; i++)
        {
            senderOutput[j][i].resize(128);
            senderOutput[j][i].set_int128(0, rot_ext.sender_output_matrices[i].squares[0].rows[j]);
        }
    }
    rot_ext.receiver_output_matrix.vertical_to(receiverOutput);
    assert(receiverOutput.size() >= key_bits.size());
    receiverOutput.resize(key_bits.size());

    if (OnlineOptions::singleton.has_option("debug_mac"))
        key.ot.check(player);
}

template<class T>
void OtMultiplier<T>::init()
{
    if (not T::has_mac)
        return;

    key_bits.set(generator.get_mac_key());
    auto& ot_setup = generator.get_mac_key().ot;
    if (OnlineOptions::singleton.has_option("debug_mac"))
        cerr << "key bits: " << key_bits.str() << "/"
                << ot_setup.base_receiver_inputs.str() << endl;
    assert(key_bits == ot_setup.base_receiver_inputs);
    senderOutput = ot_setup.base_sender_inputs.at(thread_num);
    receiverOutput = ot_setup.base_receiver_outputs.at(thread_num);

    if (OnlineOptions::singleton.has_option("debug_mac"))
    {
        cerr << "MAC key: " << generator.get_mac_key() << endl;
        ot_setup.check(*generator.players.at(thread_num));
    }

    initialize_authenticator(key_bits, senderOutput, receiverOutput);
}

template<class T>
void OtMultiplier<T>::multiply()
{
    try
    {
        multiply_raw();
    }
    catch (...)
    {
        auto filename = get_ot_secrets_filename<T>(generator.get_player());
        cerr << "Removing " << filename << endl << flush;
        unlink(filename.c_str());
        throw;
    }
}

template<class T>
void OtMultiplier<T>::multiply_raw()
{
    this->outbox.push({});
    MultJob job;
    while (this->inbox.pop(job))
    {
        if (receiverOutput.empty())
            init();

        if (job.input)
        {
            if (job.player == generator.my_num
                    or job.player == generator.players[thread_num]->other_player_num())
                multiply_for_inputs(job);
            else
                this->outbox.push(job);
        }
        else
        {
            switch (job.type)
            {
            case DATA_BIT:
                multiply_for_bits();
                break;
            case DATA_TRIPLE:
                multiply_for_triples();
                break;
            case DATA_MIXED:
                multiply_for_mixed();
                break;
            default:
                throw NotImplemented();
            }
        }
    }
}

template<class T>
void SemiMultiplier<T>::multiply_for_bits()
{
    CODE_LOCATION
    auto& rot_ext = this->rot_ext;
    auto& ot_correlator = this->ot_correlator;

    OT_ROLE role;

    if (this->generator.players[0]->my_num())
        role = SENDER;
    else
        role = RECEIVER;

    rot_ext.set_role(INV_ROLE(role));
    ot_correlator.set_role(role);

    BitVector aBits = this->generator.value_bits[0];
    rot_ext.extend(aBits.size(), aBits, not rot_ext.use_kos());

    typedef typename T::Rectangle X;
    vector<Matrix<X> >& base_sender_outputs = ot_correlator.matrices;
    Matrix<X>& base_receiver_output = ot_correlator.sender_output_matrices[0];

    rot_ext.hash_outputs(aBits.size(), base_sender_outputs, base_receiver_output,
            rot_ext.use_kos());

    int n_squares = ot_correlator.receiver_output_matrix.squares.size();
    ot_correlator.setup_for_correlation(aBits, base_sender_outputs,
            base_receiver_output);
    ot_correlator.correlate(0, n_squares, aBits, false, -1);

    c_output.clear();

    for (unsigned j = 0; j < aBits.size(); j++)
    {
        int outer = j / X::n_rows_allocated();
        int inner = j % X::n_rows_allocated();

        if (role == RECEIVER)
            c_output.push_back(
                    typename T::OpenType()
                            - ot_correlator.receiver_output_matrix.squares.at(
                                    outer).rows[inner]);
        else
            c_output.push_back(
                    ot_correlator.sender_output_matrices[0].squares.at(outer).rows[inner]);
    }

    rot_ext.set_role(BOTH);
    ot_correlator.set_role(BOTH);

    this->outbox.push({});
}

template<class T>
void SemiMultiplier<T>::multiply_for_mixed()
{
    CODE_LOCATION
    auto& rot_ext = this->rot_ext;

    typedef Square<BitVec> X;
    OtCorrelator<Matrix<X>> ot_correlator(
            this->generator.players[this->thread_num], BOTH, true);

    BitVector aBits = this->generator.value_bits[0];
    rot_ext.extend(aBits.size(), aBits, not rot_ext.use_kos());

    auto& base_sender_outputs = ot_correlator.matrices;
    auto& base_receiver_output = ot_correlator.sender_output_matrices[0];

    rot_ext.hash_outputs(aBits.size(), base_sender_outputs, base_receiver_output,
            rot_ext.use_kos());

    if (this->generator.get_player().num_players() == 2)
    {
        c_output.clear();

        for (unsigned j = 0; j < aBits.size(); j++)
        {
            this->generator.value_bits[1].set_portion(j,
                    BitVec(base_sender_outputs[0][j] ^ base_sender_outputs[1][j]));
            c_output.push_back(base_receiver_output[j] ^ base_sender_outputs[0][j]);
        }

        this->outbox.push({});
        return;
    }

    ot_correlator.setup_for_correlation(aBits, base_sender_outputs,
            base_receiver_output);
    ot_correlator.correlate(0, ot_correlator.receiver_output_matrix.squares.size(),
            this->generator.value_bits[1], false, -1);

    c_output.clear();

    for (unsigned j = 0; j < aBits.size(); j++)
    {
        c_output.push_back(
                ot_correlator.receiver_output_matrix[j]
                        ^ ot_correlator.sender_output_matrices[0][j]);
    }

    this->outbox.push({});
}

template<class W>
void OtMultiplier<W>::multiply_for_triples()
{
    CODE_LOCATION
    typedef typename W::Rectangle X;

    ot_correlator.resize(X::n_columns() * generator.n_preamp_triples_per_loop);

    rot_ext.resize(X::n_rows() * generator.n_preamp_triples_per_loop + 2 * 128);
    
    vector<Matrix<X> >& base_sender_outputs = ot_correlator.matrices;
    Matrix<X>& base_receiver_output = ot_correlator.sender_output_matrices[0];

    MultJob job;
    auto& outbox = this->outbox;
    outbox.push(job);

    bool corr_hash = generator.machine.use_extension;

    for (int i = 0; i < generator.nloops; i++)
    {
        this->inbox.pop(job);
        BitVector aBits = generator.value_bits[0];
        //timers["Extension"].start();
        if (generator.machine.use_extension)
        {
            if (rot_ext.use_kos())
                rot_ext.extend_correlated(aBits);
            else
            {
                rot_ext.extend(aBits.size(), aBits);
                corr_hash = false;
            }
        }
        else
        {
            BaseOT bot(aBits.size(), generator.players[thread_num]);
            bot.set_receiver_inputs(aBits);
            bot.exec_base(false);
            for (size_t i = 0; i < aBits.size(); i++)
            {
                rot_ext.receiver_output_matrix[i] =
                        bot.receiver_outputs[i].get_int128(0).a;
                for (int j = 0; j < 2; j++)
                    rot_ext.sender_output_matrices[j][i] =
                            bot.sender_inputs[i][j].get_int128(0).a;
            }
        }

        rot_ext.hash_outputs(aBits.size(), base_sender_outputs,
                base_receiver_output, corr_hash);
        //timers["Extension"].stop();

        //timers["Correlation"].start();
        ot_correlator.setup_for_correlation(aBits, base_sender_outputs,
                base_receiver_output);
        ot_correlator.correlate(0, generator.n_preamp_triples_per_loop,
                generator.value_bits[1], false, generator.n_amplify);
        //timers["Correlation"].stop();

        //timers["Triple computation"].start();

        this->after_correlation();
    }
}

template<class T>
void SemiMultiplier<T>::multiply_for_triples(false_type)
{
    OtMultiplier<T>::multiply_for_triples();
}

template<class T>
void SemiMultiplier<T>::multiply_for_triples(true_type)
{
    if (this->generator.machine.use_extension)
    {
        CODE_LOCATION
        MultJob job;
        this->outbox.push(job);

        for (int i = 0; i < this->generator.nloops; i++)
        {
            this->inbox.pop(job);
            const auto& a_bits = this->generator.value_bits[0];
            const auto& b_bits = this->generator.value_bits[1];
            auto tmp = a_bits;
            tmp.resize(multiple_minimum(tmp.size(), 128));
            auto ot_bits = this->rot_ext.bit_extension(tmp);
            ot_bits.second.resize(a_bits.size());
            for (auto& x : ot_bits.first)
                x.resize(a_bits.size());
            auto diff = ot_bits.first[0] ^ ot_bits.first[1] ^ b_bits;
            auto my_diff = diff;
            OctetStream os;
            diff.pack(os);
            this->generator.players[this->thread_num]->exchange(os);
            diff.unpack(os);
            assert(T::n_bits == 64);
            assert(a_bits.size() % 64 == 0);
            size_t n_chunks = a_bits.size() / 64;
            c_output.clear();
            c_output.reserve(n_chunks);
            assert(diff.size() >= ot_bits.second.size());
            for (size_t i = 0; i < n_chunks; i++)
            {
                auto c = ot_bits.second.get_word(i);
                c ^= ot_bits.first[0].get_word(i);
                c ^= a_bits.get_word(i) & diff.get_word(i);
                c_output.push_back(c);
            }
        }

        this->outbox.push(job);
    }
    else
        multiply_for_triples(false_type());
}

template<class T>
void SemiMultiplier<T>::multiply_for_triples()
{
    multiply_for_triples(T::clear::binary);
}

template <class T>
void MascotMultiplier<T>::initialize_authenticator(const BitVector& key_bits,
		const vector< array<BitVector, 2> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->auth_ot_ext.init(key_bits, senderOutput, receiverOutput);
}

template<class T>
void TinyMultiplier<T>::initialize_authenticator(const BitVector& key_bits,
        const vector<array<BitVector, 2> >& senderOutput,
        const vector<BitVector>& receiverOutput)
{
    mac_vole.init(key_bits, senderOutput, receiverOutput);
}

template <class T>
void TinierMultiplier<T>::initialize_authenticator(const BitVector& key_bits,
        const vector< array<BitVector, 2> >& senderOutput,
        const vector<BitVector>& receiverOutput)
{
    auto tmpBits = key_bits;
    tmpBits.resize_zero(128);
    auto temp_sender_output = senderOutput;
    temp_sender_output.resize(128);
    SeededPrng G;
    for (auto& x : temp_sender_output)
    {
        for (auto& y : x)
            if (y.size() == 0)
            {
                y.resize(128);
                y.randomize(G);
            }
    }
    auto temp_receiver_output = receiverOutput;
    temp_receiver_output.resize(128);
    for (auto& y : temp_receiver_output)
    {
        if (y.size() == 0)
        {
            y.resize(128);
            y.randomize(G);
        }
    }
    auth_ot_ext.init(tmpBits, temp_sender_output, temp_receiver_output);
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::initialize_authenticator(const BitVector& key_bits,
		const vector< array<BitVector, 2> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->mac_vole->init(key_bits, senderOutput, receiverOutput);
	input_mac_vole->init(key_bits, senderOutput, receiverOutput);
}

template<class T>
void SemiMultiplier<T>::after_correlation()
{
    this->ot_correlator.reduce_squares(this->generator.n_preamp_triples_per_loop,
            this->c_output);

    this->outbox.push({});
}

template <class U>
void MascotMultiplier<U>::after_correlation()
{
	typedef typename U::OpenType T;

	this->auth_ot_ext.set_role(BOTH);

    this->ot_correlator.reduce_squares(this->generator.n_preamp_triples_per_loop,
            this->c_output);

    this->outbox.push({});

    if (this->generator.machine.generate_macs)
    {
        this->macs.resize(3);
        MultJob job;
        this->inbox.pop(job);
        auto& generator = this->generator;
        array<int, 3> n_vals;
        for (int j = 0; j < 3; j++)
        {
            n_vals[j] = generator.n_triples_per_loop;
            if (this->generator.machine.check && (j % 2 == 0))
                n_vals[j] *= 2;
        }
        if (generator.machine.fewer_rounds)
        {
            BitVector bits;
            int total = 0;
            for (int j = 0; j < 3; j++)
            {
                bits.append(generator.value_bits[j],
                        n_vals[j] * T::Square::n_columns());
                total += n_vals[j];
            }
            this->auth_ot_ext.resize(bits.size());
            this->auth_ot_ext.expand(0, total);
            this->auth_ot_ext.correlate(0, total, bits, true);
            total = 0;
            for (int j = 0; j < 3; j++)
            {
                this->auth_ot_ext.reduce_squares(n_vals[j], this->macs[j], total);
                total += n_vals[j];
            }
        }
        else
        {
            this->auth_ot_ext.resize(n_vals[0] * T::Square::n_columns());
            for (int j = 0; j < 3; j++)
            {
                int nValues = n_vals[j];
                this->auth_ot_ext.expand(0, nValues);
                this->auth_ot_ext.correlate(0, nValues,
                        this->generator.value_bits[j], true);
                this->auth_ot_ext.reduce_squares(nValues, this->macs[j]);
            }
        }
        this->outbox.push(job);
    }
}

template <class T>
void TinyMultiplier<T>::after_correlation()
{
    this->ot_correlator.reduce_squares(this->generator.n_triples_per_loop,
            this->c_output);

    this->outbox.push({});

    this->macs.resize(3);
    MultJob job;
    this->inbox.pop(job);
    for (int j = 0; j < 3; j++)
    {
        int nValues = this->generator.n_triples_per_loop * T::default_length;
        auto& bits = this->generator.value_bits[j];
        vector<typename T::part_type::SacriType> values(nValues);
        for (int i = 0; i < nValues; i++)
            values[i] = bits.get_bit(i);
        mac_vole.evaluate(this->macs[j], values);
    }
    this->outbox.push(job);
}

template <class T>
void TinierMultiplier<T>::after_correlation()
{
    this->auth_ot_ext.set_role(BOTH);

    this->ot_correlator.reduce_squares(this->generator.n_triples_per_loop,
            this->c_output);

    this->outbox.push({});

    this->macs.resize(3);
    MultJob job;
    this->inbox.pop(job);
    for (int j = 0; j < 3; j++)
    {
        auth_ot_ext.expand_correlate_unchecked(this->generator.value_bits[j]);
        auth_ot_ext.transpose();
        this->macs[j].clear();
        for (size_t i = 0; i < this->generator.value_bits[j].size(); i++)
            this->macs[j].push_back(
                    Int128(
                            auth_ot_ext.receiver_output_matrix[i]
                                    ^ auth_ot_ext.sender_output_matrices[0][i]).get_lower());
    }
    this->outbox.push(job);
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::after_correlation()
{
    this->ot_correlator.reduce_squares(this->generator.n_triples_per_loop,
            this->c_output);

    this->outbox.push({});
    this->inbox.pop();

    this->macs.resize(3);
#ifdef OTCORR_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
    for (int j = 0; j < 3; j++)
    {
        int nValues = this->generator.n_triples_per_loop;
        BitVector* bits; 
        if (//this->generator.machine.check &&
                (j % 2 == 0)){
            nValues *= 2;
            bits = &(this->generator.value_bits[j]);
        }
        else {
            // piggy-backing mask after the b's
            nValues++;
            bits = &(this->generator.b_padded_bits);
        }
        this->mac_vole->evaluate(this->macs[j], nValues, *bits);
    }
#ifdef OTCORR_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCorrelated OT time: " << elapsed/1000000 << endl << flush;
#endif
    this->outbox.push({});
}

template<class T>
void MascotMultiplier<T>::multiply_for_bits()
{
    multiply_for_bits(T::clear::characteristic_two);
}

template<class T>
template<int>
void MascotMultiplier<T>::multiply_for_bits(false_type)
{
    throw runtime_error("should not be called");
}

template<class T>
template<int>
void MascotMultiplier<T>::multiply_for_bits(true_type)
{
    CODE_LOCATION
    auto& macs = this->macs;
    auto& outbox = this->outbox;
    auto& generator = this->generator;

    int nBits = generator.n_triples_per_loop + generator.field_size;
    int nBlocks = ceil(1.0 * nBits / 128);
    BitVector ext_key_bits = this->key_bits;
    if (OnlineOptions::singleton.has_option("debug_mac"))
        cerr << "key bits: " << ext_key_bits.str() << endl;
    ext_key_bits.resize_zero(128);
    auto ext_sender_output = this->senderOutput;
    ext_sender_output.resize(128, {{2, BitVector(128)}});
    SeededPrng G;
    for (auto& x : ext_sender_output)
        for (auto& y : x)
            if (y.size() < 128)
            {
                y.resize(128);
                y.randomize(G);
            }
    auto ext_receiver_output = this->receiverOutput;
    ext_receiver_output.resize(128, 128);
    OtExtensionWithMatrix auth_ot_ext(generator.players[this->thread_num], BOTH,
            true);
    auth_ot_ext.init(ext_key_bits, ext_sender_output, ext_receiver_output);
    auth_ot_ext.set_role(BOTH);
    auth_ot_ext.resize(nBlocks * 128);
    macs.resize(1);
    macs[0].resize(nBits);

    MultJob job;
    outbox.push(job);

    for (int i = 0; i < generator.nloops; i++)
    {
        auth_ot_ext.expand(0, nBlocks);
        this->inbox.pop(job);
        auth_ot_ext.correlate(0, nBlocks, generator.value_bits[0], true);
        auth_ot_ext.transpose(0, nBlocks);

        for (int j = 0; j < nBits; j++)
        {
            Int128 r = auth_ot_ext.receiver_output_matrix.squares[j/128].rows[j%128];
            Int128 s = auth_ot_ext.sender_output_matrices[0].squares[j/128].rows[j%128];
            macs[0][j] = T::clear::cut(r ^ s);
        }

        outbox.push(job);
    }
}

template<class U>
void MascotMultiplier<U>::multiply_for_inputs(MultJob job)
{
    CODE_LOCATION
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    int n_ots = job.n_inputs * generator.field_size;
    auth_ot_ext.resize(n_ots);
    auth_ot_ext.expand(0, job.n_inputs);
    if (mine)
        this->inbox.pop();
    auth_ot_ext.correlate(0, job.n_inputs, generator.value_bits[0], true);
    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    if (mine)
        for (int j = 0; j < job.n_inputs; j++)
            auth_ot_ext.receiver_output_matrix.squares[j].to(input_macs[j]);
    else
        for (int j = 0; j < job.n_inputs; j++)
        {
            auth_ot_ext.sender_output_matrices[0].squares[j].to(input_macs[j]);
            input_macs[j].negate();
        }
    this->outbox.push(job);
}

template<int K, int S>
void Spdz2kMultiplier<K, S>::multiply_for_inputs(MultJob job)
{
    CODE_LOCATION
    assert(job.input);
    bool mine = job.player == this->generator.my_num;
    input_mac_vole->set_role(mine ? SENDER : RECEIVER);
    if (mine)
        this->inbox.pop();
    input_mac_vole->evaluate(this->input_macs, job.n_inputs, this->generator.value_bits[0]);
    this->outbox.push(job);
}

template<class U>
void TinierMultiplier<U>::multiply_for_inputs(MultJob job)
{
    CODE_LOCATION
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    if (mine)
        this->inbox.pop();
    assert(not mine or job.n_inputs <= (int)generator.value_bits.at(0).size());
    auth_ot_ext.expand_correlate_unchecked(generator.value_bits[0], job.n_inputs);
    auth_ot_ext.transpose();
    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    if (mine)
        for (int j = 0; j < job.n_inputs; j++)
            input_macs[j] = Int128(auth_ot_ext.receiver_output_matrix[j]).get_lower();
    else
        for (int j = 0; j < job.n_inputs; j++)
            input_macs[j] = Int128(auth_ot_ext.sender_output_matrices[0][j]).get_lower();
    this->outbox.push(job);
}

template<class T>
void OtMultiplier<T>::multiply_for_bits()
{
    throw runtime_error("bit generation not implemented in this case");
}

template<class T>
void OtMultiplier<T>::multiply_for_mixed()
{
    throw runtime_error("mixed generation not implemented in this case");
}
