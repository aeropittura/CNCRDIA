/*
 * protocols/astra_trio/trio.h
 *
 */

#pragma once

#include "protocols/astra_trio/astra.h"
#include "protocols/astra_trio/trio_share.h"

template<class T>
class Trio : public AstraOnlineBase<T>
{
    friend class TrioShare<typename T::clear>;

    typedef AstraOnlineBase<T> Super;

    typedef typename T::OpenType OpenType;
    typedef typename T::clear clear;
    typedef pair<T, typename T::OpenType> PreTuple;

    IteratorVector<PreTuple> results;
    OctetStream prep, os[2];

    size_t n_mults();

    template<int>
    PreTuple pre_mult(const T& x, const T& y);
    template<int>
    PreTuple pre_dot(const OpenType& input);
    PreTuple pre_common(const OpenType& input);

    void initialize_reduced_mult(size_t n_mult);
    void exchange_reduced_mult(size_t n_mult);

public:
    Trio(Player& P) :
            AstraOnlineBase<T>(P)
    {
    }

    void exchange();

    T finalize_mul(int = -1) final;
    T finalize_mult_fast()
    {
        return finalize_mul();
    }

    void unsplit1(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction);
};

template<class T>
class TrioPrepProtocol : public AstraPrepProtocol<T>
{
    typedef AstraPrepProtocol<T> Super;

    typedef typename T::OpenType OpenType;

    OctetStream os, prep_os;

    void pre_p0(OpenType& r01, const OpenType& input);

public:
    TrioPrepProtocol(Player& P) :
            AstraPrepProtocol<T>(P)
    {
    }

    bool local_mult_for(int player)
    {
            return player == this->my_astra_num() and player == 0;
    }

    void exchange();

    template<class U>
    TrioPrepShare<U> from_rep3(const FixedVec<U, 2>& x);

    T get_random();
    void randoms_inst(StackedVector<T>&, const Instruction&);
};
