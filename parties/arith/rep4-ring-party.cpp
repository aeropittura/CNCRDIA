/*
 * rep4-party.cpp
 *
 */

#include "protocols/rep4/rep4_share2k.h"
#include "protocols/rep4/rep4_share.h"
#include "protocols/rep4/rep4_mc.h"
#include "numeric/z2k.h"
#include "numeric/gf2n.h"
#include "support/ez_option_parser.h"
#include "boolean/rep4_secret.h"
#include "boolean/rep4_prep.h"
#include "vm/ring_options.h"

#include "vm/ring_machine.hpp"
#include "protocols/replicated/rep_ring_only_edabit_prep.hpp"
#include "protocols/rep4/rep4_input.hpp"
#include "protocols/rep4/rep4_prep.hpp"
#include "protocols/rep4/rep4_mc.hpp"
#include "protocols/rep4/rep4.hpp"
#include "boolean/bit_adder.hpp"
#include "numeric/z2k.hpp"
#include "parties/shared/rep.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<Rep4Share2, Rep4Share>(argc, argv, 4);
}
