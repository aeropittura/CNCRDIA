/*
 * ReplicatedPrep.h
 *
 */

#ifndef GC_SEMIHONESTREPPREP_H_
#define GC_SEMIHONESTREPPREP_H_

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

    void buffer_triples() { throw not_implemented(); }
};

} /* namespace GC */

#endif /* GC_SEMIHONESTREPPREP_H_ */
