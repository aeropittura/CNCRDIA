/*
 * protocols/semi/semi_share.h
 *
 */

#pragma once

#include "protocols/common/beaver.h"
#include "protocols/semi/semi.h"
#include "vm/dummy_protocol.h"
#include "boolean/no_share.h"
#include "protocols/common/share_interface.h"

#include <string>
using namespace std;

template<class T> class Input;
template<class T> class SemiOpener;
template<class T> class DirectSemiOpener;
template<class T> class Semi;
template<class T> class Hemi;
template<class T> class SemiPrep;
template<class T> class SemiInput;
template<class T> class PrivateOutput;
template<class T> class SemiMultiplier;
template<class T> class OtTripleGenerator;
template<class T> class DummyMatrixPrep;

template<class T>
using MaybeHemi = typename conditional<T::clear::characteristic_two,
        typename T::BasicProtocol, Hemi<T>>::type;

namespace GC
{
class SemiSecret;
class NoValue;
}

template<int K> class Semi2kShare;

template<class T>
class BasicSemiShare : public T
{
public:
    typedef T OpenType;
    typedef T clear;

    typedef GC::NoValue MacKeyType;

    template<class U>
    BasicSemiShare(const U& other) : T(other)
    {
    }
};

template<class T>
class SemiShare : public T, public ShareInterface
{
    typedef T Super;
    typedef SemiShare This;

public:
    typedef T OpenType;
    typedef T clear;
    typedef SemiShare ShareType;

    typedef SemiOpener<SemiShare> MacCheck;
    typedef DirectSemiOpener<SemiShare> DirectOpener;
    typedef SemiInput<SemiShare> Input;
    typedef ::PrivateOutput<SemiShare> PrivateOutput;
    typedef Semi<SemiShare> BasicProtocol;
    typedef SemiPrep<SemiShare> LivePrep;
    typedef LivePrep TriplePrep;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    typedef SemiShare<typename T::next> PrepType;
    typedef SemiMultiplier<SemiShare> Multiplier;
    typedef OtTripleGenerator<PrepType> TripleGenerator;
    typedef T SacriType;
    typedef typename T::Square Rectangle;

#ifndef NO_MIXED_CIRCUITS
    typedef GC::SemiSecret BitType;
#endif

    const static bool needs_ot = true;
    const static bool dishonest_majority = true;
    const static bool variable_players = true;
    const static bool expensive = false;
    static const bool has_trunc_pr = true;
    static const bool malicious = false;

    static false_type is_clear;

    static string type_short() { return "D" + string(1, T::type_char()); }

    static int threshold(int nplayers)
    {
        return nplayers - 1;
    }

    static SemiShare constant(const OpenType& other, int my_num,
            MacKeyType = {}, int = -1)
    {
        if (my_num == 0)
            return other;
        else
            return {};
    }

    SemiShare()
    {
    }
    template<class U>
    SemiShare(const U& other) : T(other)
    {
    }

    void assign(const void* buffer)
    {
        Super::assign(buffer);
    }

    void pack(OctetStream& os, bool full = true) const
    {
        (void)full;
        Super::pack(os);
    }
    void unpack(OctetStream& os, bool full = true)
    {
        (void)full;
        Super::unpack(os);
    }

    void pack(OctetStream& os, int n_bits) const
    {
        Super::pack(os, n_bits);
    }
    void unpack(OctetStream& os, int n_bits)
    {
        Super::unpack(os, n_bits);
    }

    template<class U>
    static void shrsi(SubProcessor<U>& proc, const Instruction& inst)
    {
        shrsi(proc, inst, T::prime_field);
    }

    template<class U>
    static void shrsi(SubProcessor<U>&, const Instruction&,
            true_type)
    {
        throw runtime_error("shrsi not implemented");
    }

    template<class U>
    static void shrsi(SubProcessor<U>& proc, const Instruction& inst,
            false_type)
    {
        for (int i = 0; i < inst.get_size(); i++)
        {
            auto& dest = proc.get_s_ref(inst.get_r(0) + i);
            auto& source = proc.get_s_ref(inst.get_r(1) + i);
            dest = source >> inst.get_n();
        }
    }

    template<class U>
    static void split(StackedVector<U>& dest, const ArgVector& regs, int n_bits,
            const SemiShare* source, int n_inputs,
            typename U::Protocol& protocol);
};

template<class T>
false_type SemiShare<T>::is_clear;
