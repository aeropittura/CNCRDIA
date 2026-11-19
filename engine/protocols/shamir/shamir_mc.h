/*
 * protocols/shamir/shamir_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"
#include "protocols/shamir/shamir_share.h"
#include "protocols/shamir/shamir_options.h"
#include "support/bundle.h"

/**
 * Shamir secret sharing opening protocol (indirect communication)
 */
template<class T>
class IndirectShamirOpener : public MacCheckBase<T>
{
    vector<OctetStream> oss;
    OctetStream os;

public:
    IndirectShamirOpener(typename T::MacKeyType = {}, int = 0, int = 0) {}
    ~IndirectShamirOpener() {}

    virtual void exchange(const Player& P);
};

/**
 * Shamir secret sharing opening protocol (direct communication)
 */
template<class T>
class ShamirOpener : public IndirectShamirOpener<T>
{
    typedef typename T::OpenType OpenType;
    typedef typename T::OpenType::Scalar RecType;
    vector<typename T::OpenType::Scalar> reconstruction;

    ShamirOpener(const ShamirOpener&);

    void finalize(vector<typename T::OpenType>& values, const vector<T>& S);

protected:
    Bundle<OctetStream>* os;
    const Player* player;
    int threshold;

    void prepare(const vector<T>& S, const Player& P);

public:
    ShamirOpener(int threshold = 0);

    // emulate MacCheck
    ShamirOpener(const typename T::MacKeyType& _, int __ = 0, int ___ = 0) : ShamirOpener()
    { (void)_; (void)__; (void)___; }

    // emulate DirectMacCheck
    ShamirOpener(const typename T::MacKeyType& _, Names& ____, int __ = 0, int ___ = 0) :
        ShamirOpener()
    { (void)_; (void)__; (void)___; (void)____; }

    virtual ~ShamirOpener();

    void POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_Begin(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);

    virtual void initialize_open(const Player& P, int n = 0);
    virtual void prepare_open(const T& secret, int = -1);
    virtual void exchange(const Player& P);
    virtual typename T::OpenType finalize_raw();
    virtual array<typename T::OpenType*, 2> finalize_several(size_t n);

    void Check(const Player& P) { (void)P; }

    void start_exchange(const Player& P);
    void stop_exchange(const Player& P);

    vector<RecType> get_reconstruction(const Player& P, int n = 0);
    OpenType reconstruct(const vector<OpenType>& shares);

    int get_threshold() const { return threshold; }
};
