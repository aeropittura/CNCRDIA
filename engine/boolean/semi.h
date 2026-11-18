/*
 * Semi.h
 *
 */

#ifndef GC_SEMI_H_
#define GC_SEMI_H_

#include "protocols/common/beaver.h"
#include "boolean/semi_secret.h"

namespace GC
{

class SemiSecureShuffle;

class Semi : public Beaver<SemiSecret>
{
    typedef Beaver<SemiSecret> super;

public:
    Semi(Player& P) :
            super(P)
    {
    }

    void prepare_mult(const SemiSecret& x, const SemiSecret& y, int n,
            bool repeat);
    void prepare_mul(const SemiSecret& x, const SemiSecret& y, int n = -1);
};

} /* namespace GC */

#endif /* GC_SEMI_H_ */
