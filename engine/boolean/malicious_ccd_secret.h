/*
 * boolean/malicious_ccd_secret.h
 *
 */

#pragma once

#include "boolean/ccd_secret.h"
#include "boolean/malicious_ccd_share.h"

namespace GC
{

template<class T> class VectorInput;

template<class T>
class MaliciousCcdSecret : public VectorSecret<MaliciousCcdShare<T>>
{
    typedef MaliciousCcdSecret This;
    typedef VectorSecret<MaliciousCcdShare<T>> Super;

public:
    typedef TinyOpener<This> MC;
    typedef MC MacCheck;
    typedef VectorProtocol<This> Protocol;
    typedef CcdPrep<This> LivePrep;
    typedef VectorInput<This> Input;

    typedef typename This::part_type CheckType;

    static string type_short()
    {
        return "CCD";
    }

    static MC* new_opener(typename Super::MacKeyType mac_key)
    {
        return new MC(mac_key);
    }

    MaliciousCcdSecret()
    {
    }

    MaliciousCcdSecret(const typename This::part_type& other) :
            Super(other)
    {
    }

    MaliciousCcdSecret(const typename Super::Super& other) :
            Super(other)
    {
    }

    MaliciousCcdSecret(const typename This::part_type::Super& other) :
            Super(other)
    {
    }

    MaliciousCcdSecret(const GC::Clear& other) :
            Super(other)
    {
    }
};

} /* namespace GC */
