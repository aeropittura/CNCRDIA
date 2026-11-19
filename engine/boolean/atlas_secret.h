/*
 * boolean/atlas_secret.h
 *
 */

#pragma once

#include "boolean/tiny_secret.h"
#include "boolean/atlas_share.h"

namespace GC
{

class AtlasSecret : public VectorSecret<AtlasShare>
{
    typedef AtlasSecret This;
    typedef VectorSecret<AtlasShare> Super;

public:
    typedef TinyOpener<This> MC;
    typedef MC MacCheck;
    typedef VectorProtocol<This> Protocol;
    typedef VectorInput<This> Input;
    typedef CcdPrep<This> LivePrep;

    static string type_short()
    {
        return "atlas";
    }

    static MC* new_opener(typename Super::MacKeyType);

    AtlasSecret()
    {
    }

    template<class T>
    AtlasSecret(const T& other) :
            Super(other)
    {
    }
};

}
