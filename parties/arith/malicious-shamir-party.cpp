/*
 * parties/arith/malicious-shamir-party.cpp
 *
 */

#include "protocols/shamir/malicious_shamir_share.h"
#include "protocols/shamir/shamir_options.h"
#include "parties/shared/malrep.hpp"

#include "parties/shared/shamir.hpp"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<MaliciousShamirShare>(argc, argv);
}
