/*
 * protocols/rep4/rep4_mc.hpp
 *
 */

#pragma once

#include "protocols/rep4/rep4_mc.h"

template<class T>
void Rep4Opener<T>::exchange(const Player& P)
{
    CODE_LOCATION

    bool malicious = not OnlineOptions::singleton.semi_honest;

    OctetStream right, tmp;
    for (auto& secret : this->secrets)
    {
        secret[0].pack(right);
        if (malicious)
            secret[2].pack(tmp);
    }

    if (malicious)
        check_hash.update(tmp);

    P.pass_around(right, 1);
    this->values.resize(this->secrets.size());
    for (size_t i = 0; i < this->secrets.size(); i++)
    {
        typename T::OpenType a, b;
        a.unpack(right);
        this->values[i] = this->secrets[i].sum() + a;
    }

    if (malicious)
        receive_hash.update(right);

    if (OnlineOptions::singleton.has_option("always_check"))
        Check(P);
}

template<class T>
void Rep4Opener<T>::Check(const Player& P)
{
    if (check_hash.size == 0)
        return;

    CODE_LOCATION
    OctetStream left;
    check_hash.final(left);
    P.pass_around(left, -1);
    OctetStream os;
    receive_hash.final(os);
    if (os != left)
        throw MacFail();
}
