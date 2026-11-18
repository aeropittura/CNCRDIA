/*
 * boolean/rep4_prep.cpp
 *
 */

#include "boolean/rep4_prep.h"

#include "protocols/rep4/rep4.hpp"
#include "protocols/rep4/rep4_input.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/common/replicated_prep.hpp"

namespace GC
{

Rep4Prep::Rep4Prep(DataPositions& usage, int) :
        BufferPrep<Rep4Secret>(usage)
{
}

void Rep4Prep::set_protocol(Rep4Secret::Protocol& protocol)
{
    this->P = &protocol.P;
}

void Rep4Prep::buffer_bits()
{
    assert(P);
    Rep4<Rep4Secret> proto(*P);
    for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
        this->bits.push_back(proto.get_random() & 1);
}

} /* namespace GC */
