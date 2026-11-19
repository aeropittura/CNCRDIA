/*
 * boolean/malicious_rep_secret.h
 *
 */

#pragma once

#include "boolean/share_secret.h"
#include "boolean/machine.h"
#include "boolean/thread_master.h"
#include "protocols/common/beaver.h"
#include "protocols/replicated/malicious_rep_mc.h"
#include "vm/dummy_protocol.h"

template<class T> class MaliciousRepOpener;

namespace GC
{

template<class T> class ShareThread;
template<class T> class RepPrep;

class SmallMalRepSecret : public FixedVec<BitVector_<unsigned char>, 2>
{
    typedef FixedVec<BitVector_<unsigned char>, 2> Super;
    typedef SmallMalRepSecret This;

public:
    typedef MaliciousRepOpener<This> MC;
    typedef BitVector_<unsigned char> OpenType;
    typedef OpenType clear;
    typedef NoValue MacKeyType;

    static MC* new_opener(MacKeyType)
    {
        return new HashMaliciousRepOpener<This>;
    }

    SmallMalRepSecret()
    {
    }
    template<class T>
    SmallMalRepSecret(const T& other) :
            Super(other)
    {
    }

    This lsb() const
    {
        return *this & 1;
    }
};

template<class U>
class MalRepSecretBase : public ReplicatedSecret<U>
{
    typedef ReplicatedSecret<U> Super;

public:
    typedef Memory<U> DynamicMemory;

    typedef MaliciousRepOpener<U> MC;
    typedef MC MacCheck;
    typedef HashMaliciousRepOpener<U> DefaultOpener;

    typedef ReplicatedInput<U> Input;
    typedef RepPrep<U> LivePrep;

    typedef U part_type;
    typedef U WholeType;

    static const bool expensive_triples = true;
    static const bool malicious = true;

    static MC* new_opener(typename Super::MacKeyType)
    {
        try
        {
            if (ThreadMaster<U>::s().machine.more_communication_less_comp)
                return new CommunicationMaliciousRepOpener<U>;
        }
        catch(NoSingleton& e)
        {
        }
        return new HashMaliciousRepOpener<U>;
    }

    MalRepSecretBase() {}
    template<class T>
    MalRepSecretBase(const T& other) : Super(other) {}
};

class MaliciousRepSecret : public MalRepSecretBase<MaliciousRepSecret>
{
    typedef MaliciousRepSecret This;
    typedef MalRepSecretBase<This> Super;

public:
    typedef Beaver<MaliciousRepSecret> Protocol;

    typedef SmallMalRepSecret SmallType;

    MaliciousRepSecret() {}
    template<class T>
    MaliciousRepSecret(const T& other) : Super(other) {}
};

}
