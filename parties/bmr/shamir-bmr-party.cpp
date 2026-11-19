/*
 * parties/bmr/shamir-bmr-party.cpp
 *
 */

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/shamir.hpp"
#include "numeric/z2k.hpp"

int main(int argc, const char** argv)
{
	ez::EzOptionParser opt;
	ShamirOptions::singleton = {opt, argc, argv};
	RealProgramParty<ShamirShare<Gf2nLong>>(argc, argv);
}
