/*
 * protocols/spdzwise/spdzwise.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"
#include "protocols/spdzwise/spdzwise_rep3_shuffler.h"

template<class T> class SpdzWiseInput;

/**
 * Honest-majority protocol with MAC check
 */
template<class T>
class SpdzWise : public ProtocolBase<T>
{
    typedef typename T::part_type CheckType;

    friend class SpdzWiseInput<T>;

    typename T::part_type::Honest::Protocol internal, internal2;

    typename T::MacKeyType mac_key;

    vector<T> results;

    vector<typename T::part_type> coefficients;

    void buffer_random();

    virtual void zero_check(CheckType t);

public:
    typedef typename conditional<T::variable_players, SecureShuffle<T>,
            SpdzWiseRep3Shuffler<T>>::type Shuffler;

    static const bool uses_triples = false;

    Player& P;

    SpdzWise(Player& P);
    virtual ~SpdzWise();

    typename T::Protocol branch();

    void init(Preprocessing<T>&, typename T::MacCheck& MC);

    void initialize_mult();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    T finalize_mul(int n = -1);

    void initialize_dotprod();
    void prepare_dotprod(const T& x, const T& y);
    void next_dotprod();
    T finalize_dotprod(int length);

    void add_to_check(const T& x);
    void check();
    void maybe_check();

    int get_n_relevant_players() { return internal.get_n_relevant_players(); }

    void randoms_inst(StackedVector<T>& S, const Instruction& instruction);
};
