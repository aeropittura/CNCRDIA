/*
 * vm/input.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "support/buffer.h"
#include "support/time_func.h"
#include "support/pointer_vector.h"

class ArithmeticProcessor;
template<class T> class SubProcessor;
template<class T> class Preprocessing;

/**
 * Abstract base for input protocols
 */
template<class T>
class InputBase
{
    typedef typename T::clear clear;

protected:
    PlayerBase* P;
    int my_num;

    Buffer<typename T::clear, typename T::clear> buffer;
    Timer timer;

    // Send my inputs (not generally available)
    virtual void send_mine() { throw runtime_error("implement send_mine()"); }
    // Get share for next input of mine (not generally available)
    virtual T finalize_mine()
    { throw runtime_error("implement finalize_mine()"); }
    // Store share for next input from ``player`` from buffer ``o``
    // in ``target`` (not generally available)
    virtual void finalize_other(int, T&, OctetStream&, int = -1)
    { throw NotImplemented(); }

public:
    vector<OctetStream> os;
    int values_input;

    template<class U>
    static void input(SubProcessor<T>& Proc, const ArgVector& args, int size);

    static int get_player(SubProcessor<T>& Proc, int arg, bool player_from_arg);
    static void input_mixed(SubProcessor<T>& Proc, const ArgVector& args,
            int size, bool player_from_reg);
    template<class U>
    static void prepare(SubProcessor<T>& Proc, int player,
            const ArgVector::value_type* params, int size);
    template<class U>
    static void finalize(SubProcessor<T>& Proc, int player,
            const ArgVector::value_type* params, int size);

    InputBase(ArithmeticProcessor* proc = 0);
    InputBase(SubProcessor<T>* proc);
    virtual ~InputBase();

    bool virtual is_me(int player, int = -1);

    /// Initialize input round for ``player``
    virtual void reset(int player) = 0;
    /// Initialize input round for all players
    void reset_all(PlayerBase& P);

    /// Schedule input from me
    virtual void add_mine(const typename T::OpenType& input, int n_bits = -1) = 0;
    /// Schedule input from other player
    virtual void add_other(int player, int n_bits = -1) = 0;
    /// Schedule input from all players
    void add_from_all(const typename T::OpenType& input, int n_bits = -1);

    /// Run input protocol for all players
    virtual void exchange();

    /// Get share for next input from ``player``
    virtual T finalize(int player, int n_bits = -1);

    void raw_input(SubProcessor<T>& proc, const ArgVector& args, int size);
};

template<class T>
class Input : public InputBase<T>
{
    typedef typename T::OpenType OpenType;
    typedef typename T::clear clear;
    typedef typename T::MacCheck MacCheck;

    SubProcessor<T>* proc;
    MacCheck& MC;
    Preprocessing<T>& prep;
    Player& P;
    vector< PointerVector<T> > shares;
    OpenType rr, t, xi;

public:
    Input(SubProcessor<T>& proc);
    Input(SubProcessor<T>& proc, MacCheck& mc);
    Input(SubProcessor<T>* proc, Player& P);
    Input(MacCheck& MC, Preprocessing<T>& prep, Player& P);

    void reset(int player);

    void add_mine(const OpenType& input, int n_bits = -1);
    void add_other(int player, int n_bits = -1);

    void send_mine();

    T finalize_mine();
    void finalize_other(int player, T& target, OctetStream& o, int n_bits = -1);
};
