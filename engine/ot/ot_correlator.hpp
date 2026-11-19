/*
 * ot/ot_correlator.hpp
 *
 */

#pragma once

#include "ot/ot_extension_with_matrix.h"

#include "numeric/square.hpp"
#include "support/mmo.hpp"
#include "ot/bit_matrix.hpp"

template <class U>
void OtCorrelator<U>::resize(int n_ots)
{
    t1.resize_vertical(n_ots);
    u.resize_vertical(n_ots);
    sender_output_matrices.resize(2);
    for (int i = 0; i < 2; i++)
        sender_output_matrices[i].resize_vertical(n_ots);
    receiver_output_matrix.resize_vertical(n_ots);
}

template <class U>
void OtCorrelator<U>::expand(int start, int slice)
{
    (void)start, (void)slice;
    Slice<U> receiver_output_slice(receiver_output_matrix, start, slice);
    Slice<U> sender_output_slices[2] = {
            Slice<U>(sender_output_matrices[0], start, slice),
            Slice<U>(sender_output_matrices[1], start, slice)
    };
    Slice<U> t1_slice(t1, start, slice);

    // expand with PRG
    if (ot_role & RECEIVER)
    {
        for (int i = 0; i < nbase_ots; i++)
        {
            receiver_output_slice.randomize(i, g_sender[i][0]);
            t1_slice.randomize(i, g_sender[i][1]);
        }
    }

    if (ot_role & SENDER)
    {
        for (int i = 0; i < nbase_ots; i++)
            // randomize base receiver output
            sender_output_slices[0].randomize(i, g_receiver[i]);
    }
}

template <class U>
void OtCorrelator<U>::setup_for_correlation(BitVector& base_receiver_input,
        vector<U>& base_sender_outputs,
        U& base_receiver_output)
{
    this->base_receiver_input = base_receiver_input;
    receiver_output_matrix = base_sender_outputs[0];
    t1 = base_sender_outputs[1];
    u.squares.resize(t1.squares.size());
    sender_output_matrices.resize(2);
    sender_output_matrices[0] = base_receiver_output;
}

template <class U>
void OtCorrelator<U>::correlate(int start, int slice,
        BitVector& new_receiver_input, bool use_constant_base, int repeat)
{
    CODE_LOCATION
    vector<OctetStream> os(2);

    Slice<U> receiver_output_slice(receiver_output_matrix, start, slice);
    Slice<U> sender_output_slices[] = {
            Slice<U>(sender_output_matrices[0], start, slice),
    };
    Slice<U> t1_slice(t1, start, slice);
    Slice<U> u_slice(u, start, slice);

    if (OnlineOptions::singleton.has_option("verbose_correlate"))
        fprintf(stderr, "correlate %d matrices of size %d*%d, %zu bits\n", slice,
                int(U::PartType::n_rows()), int(U::PartType::n_columns()),
                new_receiver_input.size());

    // create correlation
    if (ot_role & RECEIVER)
    {
        t1_slice.rsub(receiver_output_slice);
        t1_slice.sub(new_receiver_input, repeat);
        t1_slice.pack(os[0]);

//        t1 = receiver_output_matrix;
//        t1 ^= new_receiver_input;
//        receiver_output_matrix.print_side_by_side(t1);
    }
#ifdef OTEXT_TIMER
    timeval commst1, commst2;
    gettimeofday(&commst1, NULL);
#endif
    // send t0 + t1 + x
    send_if_ot_receiver(player, os, ot_role);

    // sender adjusts using base receiver bits
    if (ot_role & SENDER)
    {
        // u = t0 + t1 + x
        u_slice.unpack(os[1]);
        sender_output_slices[0].conditional_add(base_receiver_input, u, !use_constant_base);
    }
#ifdef OTEXT_TIMER
    gettimeofday(&commst2, NULL);
    double commstime = timeval_diff(&commst1, &commst2);
    cout << "\t\tCommunication took time " << commstime/1000000 << endl << flush;
    times["Communication"] += timeval_diff(&commst1, &commst2);
#endif
}

template<class U>
void OtCorrelator<U>::expand_correlate_unchecked(const BitVector& delta, int n_bits)
{
    if (n_bits < 0)
        n_bits = delta.size();
    resize(n_bits);
    int slice = receiver_output_matrix.squares.size();
    expand(0, slice);
    BitVector tmp = delta;
    tmp.resize_zero(receiver_output_matrix.vertical_size());
    correlate(0, slice, tmp, true);
}

template <class V>
void OtExtensionWithMatrix::hash_outputs(int n_ots, vector<V>& senderOutput,
        V& receiverOutput, bool correlated)
{
    //cout << "Hashing... " << flush;
    MMO mmo;
#ifdef OTEXT_TIMER
    timeval startv, endv;
    gettimeofday(&startv, NULL);
#endif

    int n_rows = V::PartType::n_rows_allocated();
    int n = (n_ots + n_rows - 1) / n_rows * V::PartType::n_rows();
    for (int i = 0; i < 2; i++)
        senderOutput[i].resize_vertical(n);
    receiverOutput.resize_vertical(n);

    if (n_ots % 8 != 0)
        throw runtime_error("number of OTs must be divisible by 8");

    for (int i = 0; i < n_ots; i += 8)
    {
        int i_outer_input = i / 128;
        int i_inner_input = i % 128;
        int i_outer_output = i / n_rows;
        int i_inner_output = i % n_rows;
        if (ot_role & SENDER)
        {
            Int128 tmp[2][8];
            for (int j = 0; j < 8; j++)
            {
                tmp[0][j] = sender_output_matrices[0].squares[i_outer_input].rows[i_inner_input + j];
                if (correlated)
                    tmp[1][j] = tmp[0][j] ^ base_receiver_input.get_int128(0);
                else
                    tmp[1][j] =
                            sender_output_matrices[1].squares[i_outer_input].rows[i_inner_input + j];
            }
            for (int j = 0; j < 2; j++)
                mmo.hash_eight_blocks(
                        &senderOutput[j].squares[i_outer_output].rows[i_inner_output],
                        &tmp[j]);
        }
        if (ot_role & RECEIVER)
        {
            mmo.hash_eight_blocks(
                    &receiverOutput.squares[i_outer_output].rows[i_inner_output],
                    &receiver_output_matrix.squares[i_outer_input].rows[i_inner_input]);
        }
    }
    //cout << "done.\n";
#ifdef OTEXT_TIMER
    gettimeofday(&endv, NULL);
    double elapsed = timeval_diff(&startv, &endv);
    cout << "\t\tOT ext hashing took time " << elapsed/1000000 << endl << flush;
    times["Hashing"] += timeval_diff(&startv, &endv);
#endif
}

template <class U>
template <class T>
void OtCorrelator<U>::reduce_squares(unsigned int nTriples, vector<T>& output, int start)
{
    if (receiver_output_matrix.squares.size() < nTriples + start)
        throw InvalidLength("reduce_squares");
    output.resize(nTriples);
    for (unsigned int j = 0; j < nTriples; j++)
    {
#ifdef DEBUG_MASCOT
        T a, b;
        receiver_output_matrix.squares[j + start].to(a);
        sender_output_matrices[0].squares[j + start].to(b);
#endif

        receiver_output_matrix.squares[j + start].sub(
                sender_output_matrices[0].squares[j + start]).to(output[j]);

#ifdef DEBUG_MASCOT
        cout << output[j] << " ?= " << a <<  " - " << b << endl;
        cout << "first row " << receiver_output_matrix.squares[j + start].rows[0] << endl;
        assert(output[j] == a - b);
#endif
    }
}

template <class U>
void OtCorrelator<U>::common_seed(PRNG& G)
{
    Slice<U> t1_slice(t1, 0, t1.squares.size());
    Slice<U> u_slice(u, 0, u.squares.size());

    OctetStream os;
    if (player->my_num())
    {
        t1_slice.pack(os);
        u_slice.pack(os);
    }
    else
    {
        u_slice.pack(os);
        t1_slice.pack(os);
    }
    auto hash = os.hash();
    G = PRNG(hash);
}
