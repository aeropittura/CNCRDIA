/*
 * parties/arith/trio-prep-party.cpp
 *
 */

#include "protocols/astra_trio/trio_share.h"
#include "boolean/astra_secret.h"

#include "vm/machine.hpp"
#include "vm/ring_machine.hpp"
#include "protocols/astra_trio/astra_input.hpp"
#include "protocols/astra_trio/astra_prep.hpp"
#include "protocols/astra_trio/astra_mc.hpp"
#include "protocols/astra_trio/trio.hpp"
#include "boolean/share_secret.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<TrioPrepShare2, TrioPrepShare>(argc, argv);
}
