/*
 * parties/bmr/real-bmr-party.cpp
 *
 */

#define NO_MIXED_CIRCUITS

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/spdz.hpp"
#include "protocols/spdz/mascot_prep.hpp"

int main(int argc, const char** argv)
{
	RealProgramParty<Share<Gf2nLong>>(argc, argv);
}
