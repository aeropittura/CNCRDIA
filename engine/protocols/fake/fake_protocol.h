/*
 * protocols/fake/fake_protocol.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"
#include "protocols/common/secure_shuffle.h"
#include "numeric/z2k.h"
#include "vm/instruction.h"
#include "vm/trunc_pr_tuple.h"
#include "he/core/tools.h"

#include <cmath>

template<class T>
class FakeShuffle : public SecureShuffleBase<T>
{
public:
    typedef vector<vector<int>> ShuffleType;
    typedef ShuffleStore<ShuffleType> StoreType;

    map<long, long> statistics;

    FakeShuffle(SubProcessor<T>&)
    {
    }

    FakeShuffle(StackedVector<T>& a, size_t n, int unit_size, size_t output_base,
            size_t input_base, SubProcessor<T>&)
    {
        apply(a, n, unit_size, output_base, input_base, 0, 0);
    }

    void generate(size_t, ShuffleType& shuffle)
    {
        shuffle.push_back(vector<int>(1l));
    }

    void apply(StackedVector<T>& a, size_t n, size_t unit_size, size_t output_base,
            size_t input_base, size_t, bool)
    {
        auto source = a.begin() + input_base;
        auto dest = a.begin() + output_base;
        for (size_t i = 0; i < n; i++)
            // just copy
            *dest++ = *source++;

        if (n > 1)
        {
            // swap first two to pass check
            for (size_t i = 0; i < unit_size; i++)
                swap(a[output_base + i], a[output_base + i + unit_size]);
        }
    }

    void apply_multiple(StackedVector<T>& a,
            vector<ShuffleTuple<typename T::Protocol::Shuffler::ShuffleType>>& shuffles)
    {
        for (auto &shuffle : shuffles)
            this->apply(a, shuffle.size, shuffle.unit_size, shuffle.dest,
                    shuffle.source, 0, shuffle.reverse);
    }
};

template<class T>
class FakeProtocol : public ProtocolBase<T>
{
    PointerVector<T> results;
    SeededPrng G;

    T dot_prod;

    T trunc_max;

    int fails;

    vector<vector<size_t>> trunc_statistics;

    map<string, size_t> cisc_statistics;
    map<int, size_t> ltz_statistics;

public:
    typedef FakeShuffle<T> Shuffler;

    Player& P;

    FakeProtocol(Player& P) :
            fails(0), trunc_statistics(T::MAX_N_BITS + 1,
                    vector<size_t>(T::MAX_N_BITS + 1)), P(P)
    {
    }

    ~FakeProtocol()
    {
        if (not OnlineOptions::singleton.has_option("verbose_fake"))
            return;

        output_trunc_max<0>(T::invertible);
        vector<double> expected(T::MAX_N_BITS + 1);
        for (int i = 0; i <= T::MAX_N_BITS; i++)
        {
            if (sum(trunc_statistics[i]))
            {
                cerr << i << ": ";
                for (int j = 0; j <= T::MAX_N_BITS; j++)
                {
                    cerr << trunc_statistics[i][j] << " ";
                    for (int k = 0; k < T::MAX_N_BITS - i; k++)
                        expected[k] += trunc_statistics[i][j] * exp2(j - T::MAX_N_BITS);
                }
                cerr << endl;
            }
        }
        if (sum(expected) != 0)
        {
            cerr << "Expected truncation failures (log): ";
            for (size_t i = 0; i < expected.size(); i++)
            {
                auto x = expected[i];
                if (x)
                {
                    if (int(i) == OnlineOptions::singleton.trunc_error)
                        cerr << "*";
                    cerr << int(log2(x));
                    if (int(i) == OnlineOptions::singleton.trunc_error)
                        cerr << "*";
                    cerr << " ";
                }
            }
            cerr << endl;
        }
        for (auto& x : cisc_statistics)
        {
            cerr << x.second << " " << x.first << endl;
        }
        for (auto& x : ltz_statistics)
            cerr << "LTZ " << x.first << ": " << x.second << endl;
    }

    template<int>
    void output_trunc_max(false_type)
    {
        if (trunc_max != T())
            cerr << "Maximum bit length in truncation: "
                << (Bigint(typename T::clear(trunc_max)).num_bits() + 1)
                << " (" << trunc_max << ")" << endl;
    }

    template<int>
    void output_trunc_max(true_type)
    {
    }

    FakeProtocol branch()
    {
        return P;
    }

    void initialize_mult()
    {
        results.clear();
    }

    void prepare_mul(const T& x, const T& y, int = -1)
    {
        results.push_back(x * y);
    }

    void exchange()
    {
    }

    T finalize_mul(int = -1)
    {
        return results.next();
    }

    void initialize_dotprod()
    {
        initialize_mult();
        dot_prod = {};
    }

    void prepare_dotprod(const T& x, const T& y)
    {
        dot_prod += x * y;
    }

    void next_dotprod()
    {
        results.push_back(dot_prod);
        dot_prod = 0;
    }

    T finalize_dotprod(int)
    {
        return finalize_mul();
    }

    void randoms(T& res, int n_bits)
    {
        res.randomize_part(G, n_bits);
    }

    int get_n_relevant_players()
    {
        return 1;
    }

    template<int = 0>
    void trunc_pr(const vector<int>&, int, SubProcessor<T>&, true_type)
    {
        throw NotImplemented();
    }

    template<int = 0>
    void trunc_pr(const ArgVector& regs, int size, SubProcessor<T>& proc, false_type)
    {
        this->trunc_rounds++;
        for (size_t i = 0; i < regs.size(); i += 4)
            for (int l = 0; l < size; l++)
            {
                auto& res = proc.get_s_ref(regs[i] + l);
                auto& source = proc.get_s_ref(regs[i + 1] + l);
                T tmp = source;
                tmp = tmp < T() ? (T() - tmp) : tmp;
                trunc_max = max(trunc_max, tmp);
#ifdef TRUNC_PR_EMULATION_STATS
                trunc_statistics.at(regs[i + 2]).at(tmp == T() ? 0 : tmp.bit_length())++;
#endif
#ifdef CHECK_BOUNDS_IN_TRUNC_PR_EMULATION
                auto test = (source >> (regs[i + 2]));
                if (test != 0 and test != T(-1) >> regs[i + 2])
                {
                    cerr << typename T::clear(source) << " has more than "
                            << regs[i + 2]
                            << " bits in " << regs[i + 3]
                            << "-bit truncation (test value "
                            << typename T::clear(test) << ")" << endl;
                    fails++;
                    if (fails > CHECK_BOUNDS_IN_TRUNC_PR_EMULATION)
                        throw runtime_error("trunc_pr overflow");
                }
#endif
                int n_shift = regs[i + 3];
#ifdef ROUND_NEAREST_IN_EMULATION
                res = source >> n_shift;
                if (n_shift > 0)
                {
                    bool overflow = T(source >> (n_shift - 1)).get_bit(0);
                    res += overflow;
                }
#else
                if (TruncPrTupleWithGap<typename T::clear>(regs, i).big_gap())
                {
                    T r;
                    r.randomize(G);

                    if (source.negative())
                        res = -T(((-source + r) >> n_shift) - (r >> n_shift));
                    else
                        res = ((source + r) >> n_shift) - (r >> n_shift);
                    this->trunc_pr_big_counter++;

#ifdef ERROR_CHECK_IN_TRUNC_PR_EMULATION
                    T exact = tmp >> n_shift;
                    exact = source.negative() ? -exact : exact;

                    if (abs(res - exact) > 1)
                    {
                        cerr << "(" << regs[i + 2] << "," << n_shift
                                << ")-truncation failed on "
                                << tmp.bit_length()
                                << "-bit value: " << res << " vs. " << exact
                                << ", input: " << source
                                << ", randomness: " << r
                                << endl;
                        fails++;
                        if (fails > ERROR_CHECK_IN_TRUNC_PR_EMULATION)
                            throw runtime_error("trunc_pr error");
                    }
#endif
                }
                else
                {
                    T r;
                    r.randomize_part(G, n_shift);
                    if (source.negative())
                        res = -T((-source + r) >> n_shift);
                    else
                        res = (source + r) >> n_shift;
                    this->trunc_pr_counter++;
                }
#endif
            }
    }

    void cisc(SubProcessor<T>& processor, const StackedVector<Integer>& Ci,
            MemoryPart<T>& memory, const Instruction& instruction)
    {
        cisc(processor, Ci, memory, instruction, T::characteristic_two);
    }

    template<int = 0>
    void cisc(SubProcessor<T>&, const StackedVector<Integer>&,
            MemoryPart<T>&, const Instruction&, true_type)
    {
        throw NotImplemented();
    }

    template<int = 0>
    void cisc(SubProcessor<T>& processor, const StackedVector<Integer>& Ci,
            MemoryPart<T>& memory, const Instruction& instruction, false_type)
    {
        int r0 = instruction.get_r(0);
        string tag((char*)&r0, 4);
        cisc_statistics[tag.c_str()]++;
        auto& args = instruction.get_start();
        if (tag == string("LTZ\0", 4))
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                ltz_statistics[args[i + 4]] += args[i + 1];
                assert(i + args[i] <= args.size());
                assert(args[i] >= 5);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    res = T(processor.get_s()[args[i + 3] + j]).get_bit(
                            args[i + 4] - 1);
                }
            }
        }
        else if (tag == string("EQZ\0", 4))
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                assert(args[i] >= 5);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    res = processor.get_s()[args[i + 3] + j] == 0;
                }
            }
        }
        else if (tag == "Trun")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                assert(args[i] == 7);
                int k = args[i + 4];
                int m = args[i + 5];
                int s = args[i + 6];
                assert((s == 0) or (s == 1));
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    res = ((T(processor.get_s()[args[i + 3] + j])
                            + (T(s) << (k - 1))) >> m) - (T(s) << (k - m - 1));
                }
            }
        }
        else if (tag == "Mod2")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                assert(args[i] == 7);
                int m = args[i + 5];
                int s = args[i + 6];
                assert((s == 0) or (s == 1));
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    auto x = processor.get_s()[args[i + 3] + j];
                    bool sign = x.negative();
                    res = abs(x) & ((T(1) << m) - 1);
                    if (sign)
                        res = -res;
                }
            }
        }
        else if (tag == "FPDi")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                int f = args.at(i + 6);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    mpf_class a[2];
                    for (int k = 0; k < 2; k++)
                        a[k] = Bigint(typename T::clear(
                                processor.get_s()[args[i + 3 + k] + j]));
                    if (a[1] != 0)
                        res = Bigint(a[0] / a[1] * exp2(f));
                    else
                        res = 0;
                }
            }
        }
        else if (tag == "exp2")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                int f = args.at(i + 5);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    auto a = Bigint(typename T::clear(
                                    processor.get_s()[args[i + 3] + j]));
                    res = Bigint(round(exp2(mpf_class(a).get_d() / exp2(f) + f)));
                }
            }
        }
        else if (tag == "log2")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                int f = args.at(i + 5);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    auto a = Bigint(typename T::clear(
                                    processor.get_s()[args[i + 3] + j]));
                    res = Bigint(round((log2(mpf_class(a).get_d()) - f) * exp2(f)));
                }
            }
        }
        else if (tag == "bite")
        {
            for (size_t i = 0; i < args.size(); i += args[i])
            {
                assert(i + args[i] <= args.size());
                assert(args[i] == 5);
                for (size_t j = 0; j < args[i + 1]; j++)
                {
                    auto& res = processor.get_s()[args[i + 2] + j];
                    auto& source = processor.get_s()[args[i + 3] + j];
                    res = source.get_bit(Ci[args[i + 4]].get());
                }
            }
        }
        else if (tag == "sort")
        {
            assert(args[0] == args.size());
            int n = args[1];
            vector<pair<T, int>> tmp;
            tmp.reserve(n);
            auto k = processor.get_s().begin() + args[2];
            for (int i = 0; i < n; i++)
                tmp.push_back({*(k + i), i});
            stable_sort(tmp.begin(), tmp.end(), [](const auto& a, const auto& b) {
                return a.first < b.first;
            });
            vector<T> D;
            D.reserve(args[3]);
            unsigned m = args[3] / n;
            assert(n * m == args[3]);
            auto address = Ci[args[4]].get();
            auto start = memory.begin() + address;
            auto reverse = args[5];
            for (auto it = start; it < start + args[3]; it++)
                D.push_back(*it);
            for (int i = 0; i < n; i++)
            {
                int idx = tmp[i].second;
                for (unsigned j = 0; j < m; j++)
                    if (reverse)
                        memory[address + idx * m + j] = D[i * m + j];
                    else
                        memory[address + i * m + j] = D[idx * m + j];
            }
        }
        else if (tag == "Grou")
        {
            assert(args[0] == args.size());
            int n = args[1];
            auto base = processor.get_s().begin();
            auto dest = base + args[2];
            auto g = base + args[3];
            auto x = base + args[4];
            T sum;
            int last = 0;
            for (int i = 0; i < n; i++)
            {
                if ((g++)->is_one())
                {
                    for (int j = last; j < i; j++)
                        *dest++ = sum;
                    sum = *x++;
                    last = i;
                }
                else
                {
                    sum += *x++;
                }
            }
            for (int j = last; j < n; j++)
                *dest++ = sum;
            assert(dest == base + args[2] + n);
        }
        else
            throw runtime_error("unknown CISC instruction: " + tag);
    }
};
