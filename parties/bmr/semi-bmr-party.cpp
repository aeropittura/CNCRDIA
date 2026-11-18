/*
 * semi-bmr-party.cpp
 *
 */

#include "garbled/bmr/real_program_party.hpp"
#include "parties/shared/semi.hpp"

int main(int argc, const char** argv)
{
    RealProgramParty<SemiShare<gf2n_long>>(argc, argv);
}
