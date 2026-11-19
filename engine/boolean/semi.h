/*
 * boolean/semi.h
 *
 */

#pragma once

#include "protocols/common/beaver.h"
#include "boolean/semi_secret.h"

namespace GC
{

class SemiSecureShuffle;

class Semi : public Beaver<SemiSecret>
{
    typedef Beaver<SemiSecret> Super;

public:
    Semi(Player& P) :
            Super(P)
    {
    }

    void prepare_mult(const SemiSecret& x, const SemiSecret& y, int n,
            bool repeat);
    void prepare_mul(const SemiSecret& x, const SemiSecret& y, int n = -1);
};

} /* namespace GC */
