/*
 * protocols/spdzwise/spdzwise_mc.h
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep_mc.h"

template<class T>
class SpdzWiseOpener : public MacCheckBase<T>
{
    vector<typename T::OpenPartType> shares;

    void get_shares(const vector<T>& S)
    {
        shares.clear();
        for (auto& share : S)
            shares.push_back(share.get_share());
    }

public:
    typename T::OpenPartType::MacCheck inner_opener;

    SpdzWiseOpener(typename T::MacKeyType mac_key, int = 0, int = 0, int = 0) :
            MacCheckBase<T>(mac_key)
    {
    }
    SpdzWiseOpener(typename T::MacKeyType mac_key, Names&, int) :
            MacCheckBase<T>(mac_key)
    {
    }

    void initialize_open(const Player& P, int n = 0)
    {
        inner_opener.initialize_open(P, n);
    }
    void prepare_open(const T& secret, int = -1)
    {
        inner_opener.prepare_open(secret.get_share());
    }
    void exchange(const Player& P)
    {
        inner_opener.exchange(P);
    }
    typename T::OpenType finalize_raw()
    {
        return inner_opener.finalize_raw();
    }
    void Check(const Player& P)
    {
        inner_opener.Check(P);
    }
    void check_for(const typename T::OpenType& value, const vector<T>& S,
            const Player& P)
    {
        get_shares(S);
        inner_opener.check_for(value, shares, P);
    }
};
