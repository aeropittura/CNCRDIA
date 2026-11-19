/*
 * boolean/ccd_secret.h
 *
 */

#pragma once

#include "boolean/tiny_secret.h"
#include "boolean/ccd_share.h"

namespace GC
{

template<class T> class TinyOpener;
template<class T> class VectorProtocol;
template<class T> class VectorInput;
template<class T> class CcdPrep;

template<class T>
class CcdSecret : public VectorSecret<CcdShare<T>>
{
    typedef CcdSecret This;
    typedef VectorSecret<CcdShare<T>> Super;

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

    CcdSecret()
    {
    }

    CcdSecret(const typename This::part_type& other) :
            Super(other)
    {
    }

    CcdSecret(const typename Super::Super& other) :
            Super(other)
    {
    }

    CcdSecret(const typename This::part_type::Super& other) :
            Super(other)
    {
    }

    CcdSecret(const GC::Clear& other) :
            Super(other)
    {
    }
};

} /* namespace GC */
