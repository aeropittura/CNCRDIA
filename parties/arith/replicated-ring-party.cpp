/*
 * replicated-ring-party.cpp
 *
 */

#include "protocols/replicated/rep3_share2k.h"
#include "vm/ring_options.h"
#include "numeric/integer.h"
#include "parties/shared/rep_ring.hpp"
#include "vm/ring_machine.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<Rep3Share2, Rep3Share>(argc, argv);
}
