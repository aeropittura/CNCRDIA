/*
 * protocols/common/opener.h
 *
 */

#pragma once

#include <type_traits>

template<class T>
class Opener
{
    typedef typename conditional<is_same<typename T::DefaultOpener, void>::value,
            typename T::MacCheck, typename T::DefaultOpener>::type InnerType;

    InnerType inner;
    Player& P;

public:
    Opener(Player& P, typename T::MacKeyType mac_key) :
            inner(mac_key), P(P)
    {
    }

    ~Opener()
    {
        Check();
    }

    void Check()
    {
        inner.Check(P);
    }

    void initialize_open(int n = 0)
    {
        inner.initialize_open(P, n);
    }

    void prepare_open(const T& secret)
    {
        inner.prepare_open(secret);
    }

    void exchange()
    {
        inner.exchange(P);
    }

    typename T::clear finalize_open()
    {
        return inner.finalize_open();
    }
};
