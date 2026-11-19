/*
 * boolean/rep4_secret.h
 *
 */

#pragma once

#include "boolean/share_secret.h"
#include "protocols/rep4/rep4_mc.h"
#include "protocols/rep4/rep4_share.h"

namespace GC
{

class Rep4Prep;

class Rep4Secret : public RepSecretBase<Rep4Secret, 3>
{
    typedef RepSecretBase<Rep4Secret, 3> Super;
    typedef Rep4Secret This;

public:
    typedef Rep4Prep LivePrep;
    typedef Rep4<This> Protocol;
    typedef Rep4Opener<This> MC;
    typedef MC MacCheck;
    typedef Rep4Input<This> Input;

    static const bool expensive_triples = false;
    static const bool malicious = true;

    static MC* new_opener(typename Super::MacKeyType) { return new MC; }

    static This constant(const typename Super::clear& constant, int my_num,
            typename Super::MacKeyType = {}, int = -1)
    {
        return Rep4Share<typename Super::clear>::constant(constant, my_num);
    }

    Rep4Secret()
    {
    }
    template <class T>
    Rep4Secret(const T& other) :
            Super(other)
    {
    }

    void load_clear(int n, const Integer& x);
};

}
