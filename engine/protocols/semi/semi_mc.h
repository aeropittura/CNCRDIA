/*
 * protocols/semi/semi_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check.h"
#include "support/bundle.h"

/**
 * Additive secret sharing opening protocol (indirect communication)
 */
template<class T>
class SemiOpener : public TreeSum<typename T::OpenType>, public MacCheckBase<T>
{
protected:

public:
    // emulate MacCheck
    SemiOpener(const typename T::MacKeyType& _ = {}, int __ = 0, int ___ = 0)
    { (void)_; (void)__; (void)___; }
    virtual ~SemiOpener() {}

    virtual void initialize_open(const Player& P, int n = 0);
    virtual void prepare_open(const T& secret, int n_bits);
    virtual void exchange(const Player& P);

    void prepare_open(const T& secret)
    {
        this->values.push_back(secret);
    }

    void Check(const Player& P) { (void)P; }

    SemiOpener& get_part_opener() { return *this; }
};

/**
 * Additive secret sharing opening protocol (direct communication)
 */
template<class T>
class DirectSemiOpener : public SemiOpener<T>
{
    void send(const Player& P, vector<typename T::OpenType>& values);

public:
    DirectSemiOpener() {}
    // emulate DirectMacCheck
    DirectSemiOpener(const typename T::MacKeyType&, const Names& = {}, int = 0, int = 0) {}

    void POpen_(vector<typename T::OpenType>& values,const vector<T>& S,const PlayerBase& P);
    void POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P)
    { POpen_(values, S, P); }
    void POpen_Begin(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);

    void exchange(const Player& P);
    template<bool USE_LENGTHS>
    void exchange_(const PlayerBase& P);

    void start_exchange(const Player& P);

    void Check(const Player& P) { (void)P; }
};
