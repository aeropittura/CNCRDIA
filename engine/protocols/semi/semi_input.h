/*
 * protocols/semi/semi_input.h
 *
 */

#pragma once

#include "protocols/replicated/replicated_input.h"
#include "protocols/semi/hemi.h"

template<class T> class SemiOpener;

template<class T>
class PairwiseKeyInput : public PrepLessInput<T>
{
protected:
    vector<SeededPrng> send_prngs;
    vector<PRNG> receive_prngs;

public:
    PairwiseKeyInput(SubProcessor<T>* proc, PlayerBase& P);

    void maybe_initialize(PlayerBase& P);
};

/**
 * Additive secret sharing input protocol
 */
template<class T>
class SemiInput : public PairwiseKeyInput<T>
{
    PlayerBase& P;

public:
    SemiInput(SubProcessor<T>& proc, SemiOpener<T>&) :
            SemiInput(&proc, proc.P)
    {
    }

    SemiInput(SubProcessor<T>* proc, PlayerBase& P);

    SemiInput(typename T::MacCheck& MC, Preprocessing<T>& prep, Player& P,
            typename T::Protocol* = 0) :
            SemiInput(0, P)
    {
        (void) MC, (void) prep;
    }

    void reset(int player);
    void add_mine(const typename T::clear& input, int n_bits = -1);
    void add_other(int player, int n_bits = -1);
    void exchange();
    void finalize_other(int player, T& target, OctetStream& o, int n_bits = -1);
};
