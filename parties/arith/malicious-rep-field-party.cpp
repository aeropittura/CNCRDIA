/*
 * malicious-rep-field-party.cpp
 *
 */

#include "protocols/replicated/malicious_rep3_share.h"
#include "vm/field_machine.hpp"
#include "parties/shared/rep.hpp"
#include "parties/shared/malrep.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityFieldMachine<MaliciousRep3Share>(argc, argv);
}
