/*
 * vm/dummy_protocol.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "numeric/bit_vec.h"
#include "vm/data_files.h"
#include "protocols/replicated/replicated.h"
#include "protocols/common/replicated_prep.h"
#include "protocols/common/mac_check_base.h"
#include "vm/input.h"

class Player;
class DataPositions;
class ThreadQueues;

template<class T> class SubProcessor;

namespace GC
{
class NoShare;

template<class T> class ShareThread;
}

template<class T>
class DummyOpener : public MacCheckBase<T>
{
public:
    DummyOpener()
    {
    }

    template<class U>
    DummyOpener(U, int = 0, int = 0)
    {
    }

    void exchange(const Player&)
    {
        throw NotImplemented();
    }
    void check_for(const typename T::OpenType&, const vector<T>&, const Player&)
    {
    }

    DummyOpener<typename T::part_type>& get_part_opener()
    {
        return *new DummyOpener<typename T::part_type>;
    }

    typename T::MacKeyType get_alphai()
    {
        throw NotImplemented();
        return {};
    }

    int number()
    {
        return 0;
    }
};

template<class T>
class DummyProtocol : public ProtocolBase<T>
{
public:
    Player& P;
    int counter;

    static int get_n_relevant_players()
    {
        throw NotImplemented();
    }

    static void multiply(vector<T>, vector<pair<T, T>>, int, int, SubProcessor<T>)
    {
    }

    DummyProtocol(Player& P) :
            P(P)
    {
    }

    void initialize_mult()
    {
    }
    void prepare_mul(const T&, const T&, int = 0)
    {
        throw NotImplemented();
    }
    void exchange()
    {
        throw NotImplemented();
    }
    T finalize_mul(int = 0)
    {
        throw NotImplemented();
        return {};
    }
    void check()
    {
    }
};

template<class T>
class DummyLivePrep : public BufferPrep<T>
{
public:
    static const bool homomorphic = true;

    static void basic_setup(Player&)
    {
    }
    static void teardown()
    {
    }

    static void fail()
    {
        throw runtime_error(
                "live preprocessing not implemented for " + T::type_string());
    }

    static bool bits_from_dabits()
    {
        return false;
    }

    DummyLivePrep(DataPositions& usage, GC::ShareThread<T>&) :
            BufferPrep<T>(usage)
    {
    }
    DummyLivePrep(DataPositions& usage, bool = true) :
            BufferPrep<T>(usage)
    {
    }

    DummyLivePrep(SubProcessor<T>*, DataPositions& usage) :
            BufferPrep<T>(usage)
    {
    }

    void set_protocol(typename T::Protocol&)
    {
    }
    void get_three_no_count(Dtype, T&, T&, T&)
    {
        fail();
    }
    void get_two_no_count(Dtype, T&, T&)
    {
        fail();
    }
    void get_one_no_count(Dtype, T&)
    {
        fail();
    }
    void get_input_no_count(T&, typename T::OpenType&, int)
    {
        fail();
    }
    void get_no_count(StackedVector<T>&, DataTag, const ArgVector&, int)
    {
        fail();
    }
    void buffer_personal_triples(vector<array<T, 3>>&, size_t, size_t)
    {
        fail();
    }
    void buffer_personal_triples(size_t, ThreadQueues*)
    {
        fail();
    }
    void shrink_to_fit()
    {
        fail();
    }
};

template<class V>
class NotImplementedInput
{
public:
    template<class T, class U>
    NotImplementedInput(const T& proc, const U& MC)
    {
        (void) proc, (void) MC;
    }
    template<class T, class U, class W>
    NotImplementedInput(const T&, const U&, const W&)
    {
    }
    template<class T>
    NotImplementedInput(const T&)
    {
    }
    void start(int n, ArgVector regs)
    {
        (void) n, (void) regs;
        throw NotImplemented();
    }
    void stop(int n, ArgVector regs)
    {
        (void) n, (void) regs;
        throw NotImplemented();
    }
    void start(int n, int m)
    {
        (void) n, (void) m;
        throw NotImplemented();
    }
    void stop(int n, int m)
    {
        (void) n, (void) m;
        throw NotImplemented();
    }
    template<class T>
    static void input(SubProcessor<V>& proc, ArgVector regs, int)
    {
        (void) proc, (void) regs;
        throw NotImplemented();
    }
    static void input_mixed(SubProcessor<V>, ArgVector, int, int)
    {
    }
    void reset_all(Player& P)
    {
        (void) P;
        throw NotImplemented();
    }
    template<class U>
    void add_mine(U a, int b = 0)
    {
        (void) a, (void) b;
        throw NotImplemented();
    }
    void add_other(int)
    {
        throw NotImplemented();
    }
    template<class U>
    void add_from_all(U)
    {
        throw NotImplemented();
    }
    void exchange()
    {
        throw NotImplemented();
    }
    V finalize(int a, int b = 0)
    {
        (void) a, (void) b;
        throw NotImplemented();
    }
    static void raw_input(SubProcessor<V>&, ArgVector, int)
    {
        throw NotImplemented();
    }
    static void input_mixed(SubProcessor<V>&, ArgVector, int, bool)
    {
        throw NotImplemented();
    }
};

class NotImplementedOutput
{
public:
    template<class T>
    NotImplementedOutput(SubProcessor<T>& proc)
    {
        (void) proc;
    }

    void start(int player, int target, int source)
    {
        (void) player, (void) target, (void) source;
        throw NotImplemented();
    }
    void stop(int player, int source, int)
    {
        (void) player, (void) source;
    }
};
