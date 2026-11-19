/*
 * protocols/common/no_protocol.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"
#include "protocols/common/mac_check_base.h"
#include "vm/input.h"

// opening facility
template<class T>
class NoOutput : public MacCheckBase<T>
{
public:
    NoOutput(const typename T::MacKeyType& mac_key, int = 0, int = 0) :
            MacCheckBase<T>(mac_key)
    {
    }

    // open shares in this->shares and put clear values in this->values
    void exchange(const Player&)
    {
        throw runtime_error("no opening");
    }
};

// multiplication protocol
template<class T>
class NoProtocol : public ProtocolBase<T>
{
public:
    Player& P;

    static int get_n_relevant_players()
    {
        throw runtime_error("no number of relevant players");
        return -1;
    }

    NoProtocol(Player& P) :
        P(P)
    {
    }

    // prepare next round of multiplications
    void initialize_mult()
    {
    }

    // schedule multiplication
    void prepare_mul(const T&, const T&, int = -1)
    {
        throw runtime_error("no multiplication preparation");
    }

    // execute protocol
    void exchange()
    {
        throw runtime_error("no multiplication exchange");
    }

    // return next product
    T finalize_mul(int = -1)
    {
        throw runtime_error("no multiplication finalization");
    }
};

// private input facility
template<class T>
class NoInput : public InputBase<T>
{
public:
    NoInput(SubProcessor<T>&, typename T::MacCheck&)
    {
    }

    NoInput(typename T::MacCheck&, Preprocessing<T>&, Player&)
    {
    }

    NoInput(SubProcessor<T>&)
    {
    }

    // prepare next round of inputs from specific party
    void reset(int)
    {
    }

    // schedule private input from me
    void add_mine(const typename T::OpenType&, int = -1)
    {
        throw runtime_error("no input from me");
    }

    // schedule private from someone else
    void add_other(int, int = -1)
    {
        throw runtime_error("no input from others");
    }

    // send my inputs
    void send_mine()
    {
        throw runtime_error("no sending of my inputs");
    }

    // return share corresponding to my next input
    T finalize_mine()
    {
        throw runtime_error("no finalizing for my inputs");
    }

    // construct share corresponding to someone else's input
    void finalize_other(int, T&, OctetStream&, int = -1)
    {
        throw runtime_error("no finalizing of someone else's input");
    }
};
