/*
 * parties/ecdsa/mal-shamir-ecdsa-party.cpp
 *
 */

#include "protocols/shamir/malicious_shamir_share.h"

#include "protocols/shamir/shamir.hpp"
#include "protocols/shamir/shamir_input.hpp"

#include "ecdsa/hm_ecdsa_party.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ShamirOptions(opt, argc, argv);
    run<MaliciousShamirShare>(argc, argv);
}
