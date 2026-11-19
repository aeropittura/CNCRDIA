/*
 * protocols/shamir/malicious_shamir_mc.h
 *
 */

#pragma once

#include "protocols/shamir/shamir_mc.h"

/**
 * Shamir share opening with correctness check
 */
template<class T>
class MaliciousShamirOpener : public ShamirOpener<T>
{
    typedef typename T::OpenType OpenType;

    vector<vector<typename T::OpenType::Scalar>> reconstructions;

    vector<typename T::OpenType> shares;

    void finalize(vector<typename T::OpenType>& values, const vector<T>& S,
            const Player& P);

public:
    MaliciousShamirOpener();

    // emulate MacCheck
    MaliciousShamirOpener(const typename T::MacKeyType& _, int __ = 0, int ___ = 0) :
            MaliciousShamirOpener()
    { (void)_; (void)__; (void)___; }

    // emulate DirectMacCheck
    MaliciousShamirOpener(const typename T::MacKeyType& _, Names& ____, int __ = 0,
            int ___ = 0) :
            MaliciousShamirOpener()
    { (void)_; (void)__; (void)___; (void)____; }

    void initialize_open(const Player& P, int n = 0);
    typename T::OpenType finalize_raw();

    typename T::OpenType reconstruct(const vector<OpenType>& shares);
};
