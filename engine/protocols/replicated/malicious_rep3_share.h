/*
 * protocols/replicated/malicious_rep3_share.h
 *
 */

#pragma once

#include "protocols/replicated/rep3_share.h"
#include "protocols/semi/semi_share.h"

template<class T> class HashMaliciousRepOpener;
template<class T> class Beaver;
template<class T> class MaliciousRepPrepWithBits;
template<class T> class MaliciousRepPo;
template<class T> class MaliciousRepPrep;
template<class T> class SpecificPrivateOutput;

namespace GC
{
class MaliciousRepSecret;
}

template<class T>
class MaliciousRep3Share : public Rep3Share<T>
{
    typedef Rep3Share<T> Super;
    typedef MaliciousRep3Share This;

public:
    typedef Beaver<MaliciousRep3Share<T>> BasicProtocol;
    typedef HashMaliciousRepOpener<MaliciousRep3Share<T>> MacCheck;
    typedef MacCheck DirectOpener;
    typedef ReplicatedInput<MaliciousRep3Share<T>> Input;
    typedef MaliciousRepPo<MaliciousRep3Share> PO;
    typedef SpecificPrivateOutput<This> PrivateOutput;
    typedef Rep3Share<T> Honest;
    typedef MaliciousRepPrepWithBits<MaliciousRep3Share> LivePrep;
    typedef MaliciousRepPrep<MaliciousRep3Share> TriplePrep;
    typedef MaliciousRep3Share PrepType;
    typedef T random_type;
    typedef This Scalar;
    typedef MaybeHemi<This> Protocol;
    typedef DummyMatrixPrep<This> MatrixPrep;

    typedef GC::MaliciousRepSecret BitType;

    // indicate security relevance of field size
    typedef T MacKeyType;

    const static bool expensive = true;
    static const bool has_trunc_pr = false;
    static const bool malicious = true;

    static string type_short()
    {
        return "M" + string(1, T::type_char());
    }
    static string type_string()
    {
        return "malicious " + Super::type_string();
    }

    MaliciousRep3Share()
    {
    }
    template<class U>
    MaliciousRep3Share(const U& other) : Super(other)
    {
    }
};
