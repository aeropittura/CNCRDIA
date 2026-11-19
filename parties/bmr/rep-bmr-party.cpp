/*
 * parties/bmr/rep-bmr-party.cpp
 *
 */

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/rep.hpp"

int main(int argc, const char** argv)
{
    RealProgramParty<Rep3Share<Gf2nLong>>(argc, argv);
}
