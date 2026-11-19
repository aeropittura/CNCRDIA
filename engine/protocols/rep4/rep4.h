/*
 * protocols/rep4/rep4.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"

/**
 * Four-party protocol with malicious security via replication
 */
template<class T>
class Rep4 : public ProtocolBase<T>
{
    friend class Rep4RingPrep<T>;

    typedef typename T::OpenType OpenType;
    typedef array<ElementPRNG<typename T::OpenType>, 3> PrngsType;

    OctetStreams send_os;
    OctetStreams receive_os;
    vector<vector<bool>> channels;
    array<array<Hash, 4>, 4> send_hashes, receive_hashes;

    array<vector<OpenType>, 5> add_shares;
    array<OpenType, 5> dotprod_shares;
    vector<int> bit_lengths;

    class ResultTuple
    {
    public:
        T res;
        OpenType r;
    };

    PointerVector<ResultTuple> results;

    int my_num;

    bool malicious;

    array<OpenType, 5> get_addshares(const T& x, const T& y);

    void reset_joint_input(int n_inputs);
    void prepare_joint_input(int sender, int backup, int receiver,
            int outsider, vector<OpenType>& inputs);
    void prepare_joint_input(int sender, int backup, int receiver, int outsider,
            vector<OpenType>& inputs, vector<ResultTuple>& results);
    void finalize_joint_input(int sender, int backup, int receiver,
            int outsider);
    void finalize_joint_input(int sender, int backup, int receiver,
            int outsider, vector<ResultTuple>& results);

    int get_player(int offset);

    template<int = 0>
    T finalize_mul(int n_bits, true_type);
    template<int = 0>
    T finalize_mul(int n_bits, false_type);

    void must_check();

    void append_hashes(OctetStreams& to_send);
    void check_hashes(OctetStreams& to_receive);

public:
    static const bool uses_triples = false;

    PrngsType rep_prngs;
    Player& P;

    Rep4(Player& P);
    Rep4(Player& P, PrngsType& prngs);
    ~Rep4();

    Rep4 branch();

    void initialize_mult();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);
    void check();

    void initialize_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    T get_random();
    void randoms(T& res, int n_bits);

    template<int = 0>
    void trunc_pr(const ArgVector& regs, int size, SubProcessor<T>& proc,
            true_type);
    template<int = 0>
    void trunc_pr(const ArgVector& regs, int size, SubProcessor<T>& proc,
            false_type);

    template<class U>
    void split(StackedVector<T>& dest, const ArgVector& regs, int n_bits,
            const U* source, int n_inputs);

    int get_n_relevant_players() { return 2; }
};
