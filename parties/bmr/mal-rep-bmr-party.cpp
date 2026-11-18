/*
 * mal-rep-shamir-party.cpp
 *
 */

#include "protocols/replicated/malicious_rep3_share.h"

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/rep.hpp"
#include "parties/shared/malrep.hpp"

int main(int argc, const char** argv)
{
    RealProgramParty<MaliciousRep3Share<gf2n_long>>(argc, argv);
}
