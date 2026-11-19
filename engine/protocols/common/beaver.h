/*
 * protocols/common/beaver.h
 *
 */

#pragma once

#include <vector>
#include <array>
using namespace std;

#include "protocols/replicated/replicated.h"
#include "vm/data_files.h"
#include "support/code_locations.h"

template<class T> class SubProcessor;
template<class T> class MacCheckBase;
class Player;

/**
 * Beaver multiplication
 */
template<class T>
class Beaver : public ProtocolBase<T>
{
    typedef typename T::OpenType OpenType;

protected:
    vector<T> shares;
    vector<typename T::OpenType> opened;
    vector<array<T, 3>> triples;
    vector<int> lengths;
    vector<array<T, 2>> multiplicands;
    PointerVector<T> results;
    typename vector<typename T::OpenType>::iterator it;
    typename vector<array<T, 3>>::iterator triple;
    Preprocessing<T>* prep;
    typename T::MacCheck* MC;
    bool fast_mode;

    void pre_fast_exchange();
    void post_fast_exchange();

    T finalize(array<T, 3>& triple, const OpenType& eps,
            const OpenType& delta, int my_num);

public:
    static const bool uses_triples = true;

    Player& P;

    Beaver(Player& P) : prep(0), MC(0), fast_mode(false), P(P) {}

    typename T::Protocol branch();

    void set_fast_mode(bool change);

    void init(Preprocessing<T>& prep, typename T::MacCheck& MC);

    void initialize_mult();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void prepare_mult_fast(const T& x, const T& y);
    void exchange();
    T finalize_mul(int n = -1);
    T finalize_mult_fast();

    void check();

    void start_exchange();
    void stop_exchange();

    int get_n_relevant_players() { return 1 + T::threshold(P.num_players()); }

    int get_buffer_size() { return triples.size(); }
};
