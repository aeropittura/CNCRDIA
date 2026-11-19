/*
 * parties/bmr/mal-shamir-bmr-party.cpp
 *
 */

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/shamir.hpp"
#include "numeric/z2k.hpp"
#include "parties/shared/malrep.hpp"

int main(int argc, const char** argv)
{
	ez::EzOptionParser opt;
	ShamirOptions::singleton = {opt, argc, argv};
	RealProgramParty<MaliciousShamirShare<Gf2nLong>>(argc, argv);
}
