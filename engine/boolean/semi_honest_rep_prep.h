/*
 * boolean/semi_honest_rep_prep.h
 *
 */

#pragma once

#include "boolean/rep_prep.h"
#include "boolean/share_secret.h"

namespace GC
{

class SemiHonestRepPrep : public RepPrep<SemiHonestRepSecret>
{
public:
    SemiHonestRepPrep(DataPositions& usage, bool = false) :
            RepPrep<SemiHonestRepSecret>(usage)
    {
    }

    void buffer_triples() { throw NotImplemented(); }
};

} /* namespace GC */
