/*
 * bmr-program-tparty.cpp
 *
 */

#include "garbled/bmr/trusted_party.h"

int main(int argc, char** argv)
{
	TrustedProgramParty party(argc, argv);
	party.Start();
}
