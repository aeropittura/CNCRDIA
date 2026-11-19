/*
 * ot/ot_extension_with_matrix.h
 *
 */

#pragma once

#include "ot/ot_extension.h"
#include "ot/bit_matrix.h"
#include "numeric/gf2n.h"

class SoftState;

template <class U>
class OtCorrelator : public OtExtension
{
public:
    vector<U> sender_output_matrices;
    vector<U> matrices;
    U& receiver_output_matrix;
    U& t1;
    U u;

    OtCorrelator(TwoPartyPlayer* player,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OtExtension(player, role, passive),
            sender_output_matrices(2), matrices(2),
            receiver_output_matrix(matrices[0]), t1(matrices[1]) {}

    OtCorrelator(const BaseOT& base_ot, TwoPartyPlayer* player, bool passive) :
            OtExtension(base_ot, player, passive),
            sender_output_matrices(2), matrices(2),
            receiver_output_matrix(matrices[0]), t1(matrices[1]) {}

    void resize(int n_ots);
    void expand(int start, int slice);
    void setup_for_correlation(BitVector& base_receiver_input,
            vector<U>& base_sender_outputs,
            U& base_receiver_output);
    void correlate(int start, int slice, BitVector& new_receiver_input, bool use_constant_base, int repeat = 1);
    void expand_correlate_unchecked(const BitVector& delta, int n_bits = -1);
    template <class T>
    void reduce_squares(unsigned int nTriples, vector<T>& output,
            int start = 0);
    void common_seed(PRNG& G);
};

class OtExtensionWithMatrix : public OtCorrelator<BitMatrix>
{
    friend class SoftState;

    static bool warned;

    int nsubloops;

    SoftState* state;

    bool agreed;

    int softspoken_k;

    bool delete_player;

public:
    class OtBits : public pair<array<BitVector, 2>, BitVector>
    {
    public:
        OtBits(size_t size = 0)
        {
            resize(size);
        }

        void resize(size_t size)
        {
            for (auto& x : first)
                x.resize(size);
            second.resize(size);
        }

        void clear()
        {
            for (auto& x : first)
                x.resize_exact(0);
            second.resize_exact(0);
        }

        void set_part(size_t begin, const OtBits& part)
        {
            for (int k = 0; k < 2; k++)
                first[k].set_part(begin, part.first[k]);
            second.set_part(begin, part.second);
        }
    };

private:
    OtBits ot_lsbs;

    void initialize_me();

public:
    PRNG G;

    static OtExtensionWithMatrix setup(TwoPartyPlayer& player,
            Int128 delta, OT_ROLE role, bool passive);

    OtExtensionWithMatrix(
                TwoPartyPlayer* player,
                OT_ROLE role=BOTH,
                bool passive=false, int nsubloops = 1)
    : OtCorrelator<BitMatrix>(player, role, passive),
            nsubloops(nsubloops)
    {
      initialize_me();
    }

    OtExtensionWithMatrix(const BaseOT& base_ot, TwoPartyPlayer* player, bool passive);
    OtExtensionWithMatrix(Player& P, bool passive);

    ~OtExtensionWithMatrix();

    bool use_kos();
    void protocol_agreement();

    void transfer(int n_ots, const BitVector& receiver_input, int nloops);
    void extend(int n_ots, const BitVector& new_receiver_input, bool hash = true,
            bool just_bits = false);
    void extend_correlated(const BitVector& new_receiver_input);
    void extend_correlated(int n_ots, const BitVector& new_receiver_input);
    void transpose(int start = 0, int slice = -1);
    void expand_transposed();
    template <class V>
    void hash_outputs(int n_ots, vector<V>& senderOutput, V& receiverOutput,
            bool correlated = true);

    // SoftSpokenOT
    void soft_sender(size_t n_ots);
    void soft_receiver(size_t n_ots, const BitVector& new_receiver_input);

    void print(BitVector& new_receiver_input, int i = 0);
    template <class T>
    void print_receiver(BitVector& new_receiver_input, BitMatrix& matrix, int i = 0, int offset = 0);
    void print_sender(Square128& square0, Square128& square);
    template <class T>
    void print_post_correlate(BitVector& new_receiver_input, int i = 0, int offset = 0, int sender = 0);
    void print_pre_correlate(int i = 0);
    void print_post_transpose(BitVector& new_receiver_input, int i = 0,  int sender = 0);
    void print_pre_expand();

    Octet* get_receiver_output(int i);
    Octet* get_sender_output(int choice, int i);

    OtBits bit_extension(const BitVector& choices, bool multithread = false);
    void bit_extension(OtBits& res, const BitVector& choices, size_t start, size_t end);

protected:
    void hash_outputs(int n_ots);

    void check_correlation(int n_ots,
        const BitVector& receiver_input);

    void check_iteration(__m128i delta, __m128i q, __m128i q2,
        __m128i t, __m128i t2, __m128i x);
};
