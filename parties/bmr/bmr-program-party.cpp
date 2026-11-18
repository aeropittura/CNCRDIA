/*
 * bmr-program-party.cpp
 *
 */

#include "garbled/bmr/party.h"

int main(int argc, const char** argv)
{
	FakeProgramParty party(argc, argv);
	party.Start();
}
