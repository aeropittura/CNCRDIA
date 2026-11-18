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

    octetStream to_send;
    array<octetStream, 2> to_receive;

    array<PointerVector<T>, 4> results;

    array<Hash, 2> hashes;

public:
    Rep4Input(SubProcessor<T>& proc, MAC_Check_Base<T>&);
    Rep4Input(MAC_Check_Base<T>&, Preprocessing<T>&, Player& P, Rep4<T>* = 0);
    ~Rep4Input();

    void reset(int player);

    void add_mine(const typename T::open_type& input, int n_bits = -1);
    void add_other(int player, int n_bits = -1);

    void exchange();

    T finalize_mine();
    void finalize_other(int player, T& target, octetStream& o, int n_bits = -1);

    void check();
};
