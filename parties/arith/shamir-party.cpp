/*
 * parties/arith/shamir-party.cpp
 *
 */

#include "protocols/shamir/shamir_options.h"
#include "protocols/shamir/shamir_share.h"

#include "parties/shared/shamir.hpp"

int main(int argc, const char** argv)
{
    ShamirMachineSpec<ShamirShare>(argc, argv);
}
