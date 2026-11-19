/*
 * protocols/common/mac_check_base.hpp
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

template<class T>
void MacCheckBase<T>::POpen_Begin(vector<typename T::OpenType>&,
        const vector<T>& S, const Player& P)
{
    initialize_open(P, S.size());
    for (auto& secret : S)
        prepare_open(secret);
    exchange(P);
}

template<class T>
void MacCheckBase<T>::POpen_End(vector<typename T::OpenType>& values,
        const vector<T>& S, const Player&)
{
    values.clear();
    values.reserve(S.size());
    for (size_t i = 0; i < S.size(); i++)
        values.push_back(finalize_raw());
}

template<class T>
void MacCheckBase<T>::POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P)
{
    MacCheckBase<T>::POpen_Begin(values, S, P);
    MacCheckBase<T>::POpen_End(values, S, P);
}

template<class T>
typename T::OpenType MacCheckBase<T>::POpen(const T& secret, const Player& P)
{
    vector<typename T::OpenType> opened;
    POpen(opened, {secret}, P);
    return opened[0];
}

template<class T>
void MacCheckBase<T>::initialize_open(const Player&, int n)
{
    secrets.clear();
    secrets.reserve(n);
    values.clear();
    values.reserve(n);
}

template<class T>
void MacCheckBase<T>::prepare_open(const T& secret, int)
{
    secrets.push_back(secret);
}

template<class T>
typename T::clear MacCheckBase<T>::finalize_open()
{
    return finalize_raw();
}

template<class T>
typename T::OpenType MacCheckBase<T>::finalize_raw()
{
    return values.next();
}

template<class T>
array<typename T::OpenType*, 2> MacCheckBase<T>::finalize_several(size_t n)
{
    assert(values.left() >= n);
    return {{values.skip(0), values.skip(n)}};
}

template<class T>
void MacCheckBase<T>::check_for(const typename T::OpenType& value,
        const vector<T>& shares, const Player& P)
{
    vector<typename T::OpenType> opened;
    POpen(opened, shares, P);
    for (auto& check : opened)
        if (typename T::clear(value) != typename T::clear(check))
        {
            cout << check << " != " << value << endl;
            throw OfflineCheckError("CheckFor");
        }
    Check(P);
}

template<class T>
void MacCheckBase<T>::start_exchange(const Player& P)
{
    POpen_Begin(values, secrets, P);
}

template<class T>
void MacCheckBase<T>::stop_exchange(const Player& P)
{
    POpen_End(values, secrets, P);
}
