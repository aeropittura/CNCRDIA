/*
 * protocols/replicated/malicious_rep_mc.h
 *
 */

#pragma once

#include "protocols/replicated/replicated_mc.h"

template<class T>
class MaliciousRepOpener : public ReplicatedOpener<T>
{
protected:
    typedef ReplicatedOpener<T> Super;

public:
    MaliciousRepOpener(typename T::MacKeyType = {})
    {
    }

    virtual void POpen(vector<typename T::OpenType>&,
            const vector<T>&, const Player&)
    {
        throw runtime_error("use subclass");
    }

    virtual void POpen_Begin(vector<typename T::OpenType>& values,
            const vector<T>& S, const Player& P);

    virtual void POpen_End(vector<typename T::OpenType>&,
            const vector<T>&, const Player&)
    {
        throw runtime_error("use subclass");
    }

    virtual void Check(const Player&)
    {
        throw runtime_error("use subclass");
    }

    MaliciousRepOpener& get_part_opener()
    {
        return *this;
    }
};

/**
 * 3-party replicated opening with checking via hash
 */
template<class T>
class HashMaliciousRepOpener : public MaliciousRepOpener<T>
{
    Hash hash;

    OctetStream os;

    bool needs_checking;

    vector<OctetStream> to_send, to_receive;
    OctetStream hash_buffer;

    void reset();
    void update();

    void prepare(const vector<T>& secrets, const Player& P);
    void finalize(vector<typename T::OpenType>& values,
            const vector<T>& secrets, const Player& P);

public:
    // emulate MacCheck
    HashMaliciousRepOpener(const typename T::MacKeyType& _, int __ = 0, int ___ = 0) : HashMaliciousRepOpener()
    { (void)_; (void)__; (void)___; }

    // emulate DirectMacCheck
    HashMaliciousRepOpener(const typename T::MacKeyType& _, Names& ____, int __ = 0, int ___ = 0) : HashMaliciousRepOpener()
    { (void)_; (void)__; (void)___; (void)____; }

    HashMaliciousRepOpener();
    ~HashMaliciousRepOpener();

    void POpen(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_Begin(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);
    void POpen_End(vector<typename T::OpenType>& values,const vector<T>& S,const Player& P);

    virtual void exchange(const Player& P);
    virtual typename T::OpenType finalize_raw();

    void check_for(const typename T::OpenType& value, const vector<T>& shares, const Player& P);

    void Check(const Player& P);
};

template<class T>
class CommunicationMaliciousRepOpener : public MaliciousRepOpener<T>
{
    vector<OctetStream> os;

public:
    void POpen(vector<typename T::clear>& values, const vector<T>& S,
            const Player& P);
    void POpen_Begin(vector<typename T::clear>& values, const vector<T>& S,
            const Player& P);
    void POpen_End(vector<typename T::clear>& values, const vector<T>& S,
            const Player& P);

    void Check(const Player& P);
};
