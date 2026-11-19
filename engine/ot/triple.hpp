/*
 * ot/triple.hpp
 *
 */

#pragma once

template<class T> class NPartyTripleGenerator;

template <class T, int N>
class Triple
{
public:
    T a[N];
    T b;
    T c[N];

    int repeat(int l, bool check)
    {
        switch (l)
        {
        case 0:
        case 2:
            if (check)
                return N;
            else
                return 1;
        case 1:
            return 1;
        default:
            throw BadValue();
        }
    }

    T& by_index(int l, int j)
    {
        switch (l)
        {
        case 0:
            return a[j];
        case 1:
            return b;
        case 2:
            return c[j];
        default:
            throw BadValue();
        }
    }

    template <int M>
    void amplify(const Triple<T,M>& unchecked_triple, PRNG& G)
    {
        b = unchecked_triple.b;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
            {
                typename T::value_type r;
                r.randomize(G);
                a[i] += r * unchecked_triple.a[j];
                c[i] += r * unchecked_triple.c[j];
            }
    }

    void output(ostream& output_stream, int n = N, bool human = false)
    {
        for (int i = 0; i < n; i++)
        {
            a[i].output(output_stream, human);
            b.output(output_stream, human);
            c[i].output(output_stream, human);
        }
    }

    template<class V>
    Triple<V, 1> reduce()
    {
        Triple<V, 1> triple;
        for (int i = 0; i < 3; i++)
            triple.by_index(i, 0) = by_index(i, 0);
        return triple;
    }
};

template <class T, int N>
class PlainTriple : public Triple<T,N>
{
public:
    // this assumes that value_bits[1] is still set to the bits of b
    void to(vector<BitVector>& value_bits, int i, int repeat = N)
    {
        for (int j = 0; j < N; j++)
        {
            value_bits[0].set_portion(i * repeat + j, this->a[j]);
            value_bits[2].set_portion(i * repeat + j, this->c[j]);
        }
    }
};

// T is Z2<K + 2S>, U is Z2<K + S>
template <class T, class U, int N>
class PlainTriple_ : public PlainTriple<T,N>
{
public:

    template <int M>
    void amplify(BitVector& a, U& b, Rectangle<Z2<M>, U>& c, PRNG& G)
    {
        assert(a.size() == M);
        this->b = b;
        for (int i = 0; i < N; i++)
        {
            U aa = 0, cc = 0;
            for (int j = 0; j < M; j++)
            {
                U r;
                r.randomize(G);
                if (a.get_bit(j))
                    aa += r;
                cc += U::Mul(r, c.rows[j]);
            }
            this->a[i] = aa;
            this->c[i] = cc;
        }
    }
};

template <class U, int N>
class ShareTriple : public Triple<U, N>
{
    typedef typename U::OpenType T;

public:
    template<class V>
    void from(PlainTriple<T,N>& triple,
            int i_triple, const NPartyTripleGenerator<V>& generator)
    {
        for (int l = 0; l < 3; l++)
        {
            int repeat = this->repeat(l, generator.machine.check);
            for (int j = 0; j < repeat; j++)
            {
                typename U::ShareType value = triple.by_index(l,j);
                typename U::MacType mac;
                mac = (value * generator.get_mac_key());
                for (int i = 0; i < generator.nparties-1; i++)
                    mac += generator.ot_multipliers[i]->macs.at(l).at(i_triple * repeat + j);
                U& share = this->by_index(l,j);
                share.set_share(value);
                share.set_mac(mac);
            }
        }
    }
};

template <class T>
class TripleToSacrifice : public Triple<T, 1>
{
public:
    template <class W>
    void prepare_sacrifice(const Triple<T, 2>& unchecked_triple, PRNG& G)
    {
        this->b = unchecked_triple.b;
        typename W::MacKeyType t;
        t.randomize(G);
        this->a[0] = unchecked_triple.a[0] * t - unchecked_triple.a[1];
        this->c[0] = unchecked_triple.c[0] * t - unchecked_triple.c[1];
    }

    T compute_check_share(const typename T::OpenType& masked_a)
    {
        return this->c[0] - masked_a * this->b;
    }
};
