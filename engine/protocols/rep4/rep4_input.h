/*
 * protocols/rep4/rep4_input.h
 *
 */

#pragma once

#include "protocols/replicated/replicated_input.h"

template<class T>
class Rep4Input : public InputBase<T>
{
    Rep4<T> protocol;
    Player& P;

    OctetStream to_send;
    array<OctetStream, 2> to_receive;

    array<PointerVector<T>, 4> results;

    array<Hash, 2> hashes;

public:
    Rep4Input(SubProcessor<T>& proc, MacCheckBase<T>&);
    Rep4Input(MacCheckBase<T>&, Preprocessing<T>&, Player& P, Rep4<T>* = 0);
    ~Rep4Input();

    void reset(int player);

    void add_mine(const typename T::OpenType& input, int n_bits = -1);
    void add_other(int player, int n_bits = -1);

    void exchange();

    T finalize_mine();
    void finalize_other(int player, T& target, OctetStream& o, int n_bits = -1);

    void check();
};
