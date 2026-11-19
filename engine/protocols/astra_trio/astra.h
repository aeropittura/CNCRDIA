/*
 * protocols/astra_trio/astra.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"

template<class T> class TrioPrepShare;
template<class T> class AstraPrepShare;
template<class T> class AstraShare;
template<class T> class AstraPrepInput;

template <class T>
class AstraBase : public ProtocolBase<T>
{
    friend class AstraShare<typename T::clear>;

protected:
    vector<typename T::OpenType> inputs;
    vector<array<T, 2>> input_pairs;
    IteratorVector<T> results;
    size_t n_mults;

    string suffix;

    // for trunc_pr
    static const int gen_player = 0;
    static const int comp_player = 1;

    IteratorVector<T> gen_values;

    string get_filename(bool create, const char* name = "Protocol");
    string get_output_filename();

    void debug();

    virtual int my_astra_num() = 0;

    virtual bool local_mult_for(int player) = 0;
    void prepare_exchange();

    virtual void initialize_reduced_mult(size_t n_mult) = 0;
    virtual void exchange_reduced_mult(size_t n_mult) = 0;

    virtual void initialize_input0(size_t n_mult) = 0;
    virtual void exchange_input0(size_t n_mult) = 0;
    virtual void finalize_input0(size_t n_mult) = 0;

    template<int MY_NUM>
    void trunc_pr_small_gap_finish(const TruncPrTupleList<T>& infos,
            int size, SubProcessor<T>& proc);

public:
    Player& P;

    AstraBase(Player& P);
    virtual ~AstraBase();

    virtual void initialize_prep() = 0;

    /// Initialize multiplication round
    void initialize_mult();
    void prepare_mul(const T& x, const T& y, int n = -1) final;
    void prepare_mult_fast(const T& x, const T& y)
    {
        prepare_mul(x, y);
    }

    /// Initialize dot product round
    void initialize_dotprod();
    /// Add operand pair to current dot product
    void prepare_dotprod(const T& x, const T& y);
    /// Finish dot product
    void next_dotprod();
    /// Get next dot product result
    T finalize_dotprod(int length);

    template<int = 0>
    void trunc_pr_small_gap(const TruncPrTupleList<T>& infos, int size,
            SubProcessor<T>& proc);

    int get_n_relevant_players()
    {
        return 2;
    }

    void set_suffix(const string& suffix);

    Player& branch()
    {
        throw NotImplemented();
    }
};

template<class T>
class AstraOnlineBase : public AstraBase<T>
{
    typedef typename T::OpenType OpenType;
    typedef OpenType value_type;

protected:
    ifstream prep;
    ofstream outputs;
    int astra_num;

    OctetStream cs_prep;

    int my_astra_num()
    {
        return astra_num;
    }

    bool local_mult_for(int player)
    {
        return player == my_astra_num();
    }

    void initialize_input0(size_t)
    {
    }

    void exchange_input0(size_t);

    void finalize_input0(size_t)
    {
    }

public:
    AstraOnlineBase(Player& P);

    void initialize_prep();

    template<class U>
    U read();
    void read(OctetStream& os);

    template<class U>
    void sync(vector<U>& values, Player& P);
    template<class U>
    void forward_sync(vector<U>& values);

    T get_random();
    void randoms_inst(StackedVector<T>&, const Instruction&);

    template<int = 0>
    void trunc_pr(const ArgVector& regs, int size, SubProcessor<T>& proc,
            false_type);

    template<int = 0>
    void trunc_pr_big_gap(const TruncPrTupleList<T>& infos, int size,
            SubProcessor<T>& proc);

    void unsplit(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction);
    virtual void unsplit1(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction) = 0;
};

template<class T>
class Astra : public AstraOnlineBase<T>
{
    friend class AstraShare<typename T::clear>;

    typedef typename T::OpenType OpenType;
    typedef typename T::clear clear;
    typedef OpenType value_type;

    OctetStream os, os_prep, recv_os;

    T pre(const OpenType& input);

    void initialize_reduced_mult(size_t n_mult);
    void exchange_reduced_mult(size_t n_mult);

    void unsplit1(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction);

public:
    Astra(Player& P);

    /// Run multiplication protocol
    void exchange();
    /// Get next multiplication result
    T finalize_mul(int n = -1) final;
    T finalize_mult_fast()
    {
        return finalize_mul();
    }
};

template<class T>
class AstraPrepProtocol : public AstraBase<T>
{
    friend class AstraPrepInput<T>;
    friend class AstraPrepShare<typename T::clear>;
    friend class TrioPrepShare<typename T::clear>;
    friend class ReplicatedInput<T>;

    typedef typename T::OpenType OpenType;
    typedef OpenType value_type;

    OctetStream os_prep;
    OctetStream cs, cs_prep;

    ReplicatedInput<Rep3Share<typename T::clear>>* unsplit_input;

    template<int MY_NUM>
    void unsplit_finish(StackedVector<T>& dest, const Instruction& instruction);

    void initialize_reduced_mult(size_t n_mult);
    void exchange_reduced_mult(size_t n_mult);

    void initialize_input0(size_t n_mult);
    void exchange_input0(size_t n_mult);
    void finalize_input0(size_t n_mult);

protected:
    ofstream prep;
    ifstream outputs;
    ReplicatedBase prng_protocol;
    ReplicatedBase prng_protocol_for_input0;
    int my_num;
    OctetStream os;

    int my_astra_num()
    {
        return my_num;
    }

    int rep_index(int i);
    int rep_index(int i, int my_num);

    void add_gen(OctetStream& cs, const typename T::OpenType& value);

    template<int my_num>
    void pre();
    template<int my_num>
    void pre_element(T& res);
    template<int my_num>
    void pre_gamma(T& res, OpenType& gamma, const OpenType& input);

    void post(T& res, const OpenType& gamma);

public:
    AstraPrepProtocol(Player& P);
    ~AstraPrepProtocol();

    void initialize_prep();

    virtual bool local_mult_for(int player)
    {
        return player == my_astra_num();
    }

    /// Run multiplication protocol
    void exchange();

    T finalize_mul(int n = -1) final;
    T finalize_mult_fast()
    {
        return finalize_mul();
    }

    template<class U>
    void store(const U& value);
    void store(const OctetStream& os);

    template<class U>
    void sync(vector<U>& values, Player& P);
    template<class U>
    void forward_sync(vector<U>& values);

    T get_random();
    void randoms_inst(StackedVector<T>&, const Instruction&);

    template<int = 0>
    void trunc_pr(const ArgVector& regs, int size, SubProcessor<T>& proc,
            false_type);

    template<int = 0>
    void trunc_pr_big_gap(const TruncPrTupleList<T>& infos, int size,
            SubProcessor<T>& proc);

    template<class U>
    AstraPrepShare<U> from_rep3(const FixedVec<U, 2>& x);
    template<class U>
    AstraPrepShare<U> from_rep3(const FixedVec<U, 2>& x, int my_num);

    void unsplit(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction);
    void unsplit1(StackedVector<T>& dest,
            StackedVector<typename T::BitType>& source,
            const Instruction& instruction);
};
