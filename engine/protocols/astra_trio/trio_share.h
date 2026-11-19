/*
 * protocols/astra_trio/trio_share.h
 *
 */

#pragma once

#include "protocols/astra_trio/astra_share.h"

template<class T> class Trio;
template<class T> class TrioPrepProtocol;
template<class T> class TrioOpener;
template<class T> class TrioInput;

template<class T>
class TrioShare : public AstraShare<T>
{
    typedef TrioShare This;
    typedef AstraShare<T> Super;

public:
    // type for clear values in relevant domain
    typedef T clear;
    typedef clear OpenType;

    typedef GC::AstraSecret<TrioShare<BitVec>> BitType;

    // opening facility
    typedef TrioOpener<This> MacCheck;
    typedef MacCheck DirectOpener;

    // multiplication protocol
    typedef Trio<This> Protocol;

    // preprocessing facility
    typedef AstraPrep<This> LivePrep;

    // private input facility
    typedef TrioInput<This> Input;

    // default private output facility (using input tuples)
    typedef ::PrivateOutput<This> PrivateOutput;

    template<class U>
    using OpenerTemplate = TrioOpener<U>;
    template<class U>
    using ProtocolTemplate = Trio<U>;
    template<class U>
    using InputTemplate = TrioInput<U>;

    static string type_string()
    {
        return "Trio share " + T::type_string();
    }

    // used for preprocessing storage location
    static string type_short()
    {
        return "trio-" + T::type_short();
    }

    TrioShare()
    {
    }

    template<class U>
    TrioShare(const U& other) :
            Super(other)
    {
    }

    T local_mul_P1(const This& other) const;
    T local_mul_P2(const This& other) const;

    T for_split(int i) const
    {
        if (i == 0)
            return common_m();
        else
            return (*this)[i];
    }

    T common_m() const
    {
        return this->m(-1) - this->neg_lambda(-1);
    }

    void set_common_m(const T& x)
    {
        (*this)[0] = x + (*this)[1];
    }

    FixedVec<T, 2> to_rep3() const
    {
        This res;
        for (int i = 0; i < 2; i++)
            res[i] = for_split(i);
        return res;
    }

    static This from_rep3(const FixedVec<T, 2>& x)
    {
        This res(x);
        res[0] += res[1];
        return res;
    }

    static This from_astra(FixedVec<T, 2>& x)
    {
        return from_rep3(x);
    }

    template<class U, int MY_NUM>
    static void pre_reduced_mult(U& a, U& b, U& c, Trio<U>& protocol,
            const T& aa, const T& bb);

    template<class U, int MY_NUM>
    static pair<U, clear> post_reduced_mult(typename U::Protocol & protocol);

    template<class U, int MY_NUM>
    static This post_input0(typename U::Protocol& protocol);

    template<class U>
    static void shrsi(SubProcessor<U>& proc, const Instruction& inst)
    {
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_s_ref(inst.get_r(0) + i);
            auto& source = proc.get_s_ref(inst.get_r(1) + i);
            dest = from_rep3(source.to_rep3() >> inst.get_n());
        }
    }
};

template<class T>
class TrioPrepShare : public AstraPrepShare<T>
{
    typedef TrioPrepShare This;
    typedef AstraPrepShare<T> Super;

public:
    typedef GC::AstraSecret<TrioPrepShare<BitVec>> BitType;

    // opening facility
    typedef ReplicatedOpener<This> MacCheck;
    typedef MacCheck DirectOpener;

    // multiplication protocol
    typedef TrioPrepProtocol<This> Protocol;

    // preprocessing facility
    typedef AstraPrepPrep<This> LivePrep;

    // private input facility
    typedef AstraPrepInput<This> Input;

    // default private output facility (using input tuples)
    typedef ::PrivateOutput<This> PrivateOutput;

    template<class U>
    using ProtocolTemplate = TrioPrepProtocol<U>;

    // used for preprocessing storage location
    static string type_short()
    {
        return "trio-" + T::type_short();
    }

    TrioPrepShare()
    {
    }

    template<class U>
    TrioPrepShare(const U& other) :
            Super(other)
    {
    }

    T local_mul_P0(const This& other) const;
    T local_mul_P1(const This& other) const;
    T local_mul_P2(const This& other) const;
};

template<int K>
using TrioShare2 = TrioShare<SignedZ2<K>>;
template<int K>
using TrioPrepShare2 = TrioPrepShare<SignedZ2<K>>;
