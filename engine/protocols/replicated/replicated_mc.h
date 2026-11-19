/*
 * protocols/replicated/replicated_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"

/**
 * Replicated semi-honest three-party opening protocol
 */
template <class T>
class ReplicatedOpener : public MacCheckBase<T>
{
    OctetStream o;
    OctetStream to_send;

    void prepare(const vector<T>& S);
    void finalize(vector<typename T::OpenType>& values, const vector<T>& S);

public:
    // emulate MacCheck
    ReplicatedOpener(const typename T::MacKeyType& _ = {}, int __ = 0, int ___ = 0)
    { (void)_; (void)__; (void)___; }

    // emulate DirectMacCheck
    ReplicatedOpener(const typename T::MacKeyType& _, Names& ____, int __ = 0, int ___ = 0)
    { (void)_; (void)__; (void)___; (void)____; }

    void POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_Begin(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);

    virtual void exchange(const Player& P);
    virtual typename T::OpenType finalize_raw();
    virtual array<typename T::OpenType*, 2> finalize_several(size_t n);

    void Check(const Player& P) { (void)P; }

    ReplicatedOpener& get_part_opener()
    {
        return *this;
    }
};
