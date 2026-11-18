/*
 * Rep4Prep.h
 *
 */

#ifndef GC_REP4PREP_H_
#define GC_REP4PREP_H_

#include "boolean/rep4_secret.h"
#include "boolean/personal_prep.h"
#include "protocols/common/replicated_prep.h"

namespace GC
{

class Rep4Prep : public BufferPrep<Rep4Secret>
{
public:
    Rep4Prep(DataPositions& usage, int _ = -1);

    void set_protocol(Rep4Secret::Protocol& protocol);

    void buffer_bits();
};

} /* namespace GC */

#endif /* GC_REP4PREP_H_ */
