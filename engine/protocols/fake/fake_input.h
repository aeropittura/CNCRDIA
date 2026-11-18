/*
 * protocols/fake/fake_input.h
 *
 */

#pragma once

#include "protocols/replicated/replicated.h"
#include "vm/input.h"

template<class T>
class FakeInput : public InputBase<T>
{
    PointerVector<T> results;

public:
    FakeInput()
    {
    }

    FakeInput(SubProcessor<T>&, typename T::MAC_Check&)
    {
    }

    void reset(int)
    {
        results.clear();
    }

    void add_mine(const typename T::open_type& x, int = -1)
    {
        results.push_back(x);
    }

    void add_other(int, int = -1)
    {
    }

    void send_mine()
    {
    }

    T finalize_mine()
    {
        return results.next();
    }

    void finalize_other(int, T&, octetStream&, int = -1)
    {
        throw runtime_error("input from other parties not implemented");
    }
};
