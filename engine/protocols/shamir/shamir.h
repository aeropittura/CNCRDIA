/*
 * protocols/shamir/shamir.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "protocols/replicated/replicated.h"

template<class T> class SubProcessor;
template<class T> class ShamirOpener;
template<class T> class ShamirShare;
template<class T> class ShamirInput;
template<class T> class IndirectShamirOpener;

class Player;

/**
 * Shamir secret sharing-based protocol with resharing
 */
template<class T>
class Shamir : public ProtocolBase<T>
{
    typedef typename T::OpenType::Scalar U;

    vector<U> reconstruction;
    U rec_factor;
    ShamirInput<T>* resharing;
    ShamirInput<T>* random_input;

    SeededPrng secure_prng;

    map<int, vector<vector<typename T::OpenType>>> hypers;

    typename T::OpenType dotprod_share;

    void buffer_random();

    int threshold;
    int n_mult_players;

public:
    static const bool uses_triples = false;

    Player& P;

    static U get_rec_factor(int i, int n);
    static U get_rec_factor(int i, const vector<int>& points, int target = -1);
    static vector<U> get_rec_factors(const vector<int>& points, int target = -1);

    Shamir(Player& P, int threshold = 0);
    ~Shamir();

    Shamir branch();

    int get_n_relevant_players();

    void reset();

    void initialize_mult();

    void prepare_mul(const T& x, const T& y, int n = -1);

    void exchange();
    void start_exchange();
    void stop_exchange();

    T finalize_mul(int n = -1);

    T finalize(int n_input_players);

    void initialize_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    vector<T> get_randoms(PRNG& G, int t);

    vector<vector<typename T::OpenType>>& get_hyper(int t);
    static void get_hyper(vector<vector<typename T::OpenType>>& hyper, int t, int n);
    static string hyper_filename(int t, int n);
};
