/*
 * protocols/replicated/post_sacrifice.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"

/**
 * Protocol with optimistic multiplication and postponed sacrifice
 */
template<class T>
class PostSacrifice : public ProtocolBase<T>
{
    typedef typename T::PrepType PrepType;

    typename PrepType::Honest::Protocol internal;
    typename T::Honest::Protocol randomizer;

    vector<array<PrepType, 2>> operands;
    vector<PrepType> results;

public:
    Player& P;

    PostSacrifice(Player& P);
    ~PostSacrifice();

    Player& branch();

    void initialize_mult();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange() { internal.exchange(); }
    T finalize_mul(int n = -1);

    void check();

    int get_n_relevant_players() { return internal.get_n_relevant_players(); }

    virtual void randoms(T& res, int n_bits) { randomizer.randoms(res, n_bits); }
};
