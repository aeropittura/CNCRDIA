/*
 * protocols/common/edabit.h
 *
 */

#pragma once

#include "support/fixed_vector.h"

template<class T>
using Edabit = pair<T, FixedVector<typename T::BitType::part_type::SmallType, T::clear::MAX_EDABITS + 5>>;

template<class T>
class Edabitvec
{
    typedef FixedVector<typename T::BitType::part_type, T::clear::MAX_EDABITS + 5> BType;
    typedef FixedVector<T, T::BitType::part_type::default_length> AType;

public:
    static const int MAX_SIZE = AType::MAX_SIZE;

    AType a;
    BType b;

    Edabitvec()
    {
    }

    Edabitvec(const Edabit<T>& other) :
            Edabitvec(other.first, other.second)
    {
    }

    Edabitvec(const T& aa, const typename Edabit<T>::second_type& bb)
    {
        a.push_back(aa);
        for (auto& x : bb)
            b.push_back(x);
    }

    Edabitvec(const vector<typename T::BitType::part_type>& other)
    {
        b = other;
    }

    void push_a(const T& x)
    {
        a.push_back(x);
    }

    bool empty()
    {
        return a.empty();
    }

    bool full()
    {
        return a.full();
    }

    size_t size()
    {
        return a.size();
    }

    T get_a(int i)
    {
        return a[i];
    }

    typename T::BitType get_b(int i)
    {
        return b[i];
    }

    Edabit<T> next()
    {
        Edabit<T> res;
        res.first = a.pop();
        if (T::BitType::part_type::default_length > 1)
            for (auto& x : b)
                res.second.push_back(x.get_bit(a.size()));
        else
            for (auto& x : b)
                res.second.push_back(x);
        return res;
    }

    void push_back(const Edabit<T>& x)
    {
        for (size_t i = 0; i < x.second.size(); i++)
        {
            b[i] ^= (typename T::BitType::part_type(x.second[i])
                    ^ b[i].get_bit(a.size())) << a.size();
        }
        a.push_back(x.first);
    }

    void input(int length, ifstream& s)
    {
        char* buffer = new char[MAX_SIZE * T::size()];
        s.read(buffer, MAX_SIZE * T::size());
        for (int i = 0; i < MAX_SIZE; i++)
        {
            T x;
            x.assign(buffer + i * T::size());
            a.push_back(x);
        }
        size_t bsize = T::BitType::part_type::size();
        char* bbuffer = new char[length * bsize];
        s.read(bbuffer, length * bsize);
        for (int i = 0; i < length; i++)
        {
            typename T::BitType::part_type x;
            x.assign(bbuffer + i * bsize);
            b.push_back(x);
        }
        delete[] bbuffer;
        delete[] buffer;
    }

    void output(int length, ofstream& s)
    {
        assert(size() == MAX_SIZE);
        for (auto& x : a)
            x.output(s, false);
        for (int i = 0; i < length; i++)
            b[i].output(s, false);
    }

    void pack(OctetStream& os) const
    {
        a.pack(os);
        b.pack(os);
    }

    void unpack(OctetStream os)
    {
        a.unpack(os);
        b.unpack(os);
    }
};
