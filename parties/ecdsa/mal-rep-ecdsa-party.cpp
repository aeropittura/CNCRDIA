/*
 * parties/ecdsa/mal-rep-ecdsa-party.cpp
 *
 */

#include "protocols/replicated/malicious_rep3_share.h"

#include "ecdsa/hm_ecdsa_party.hpp"

int main(int argc, const char** argv)
{
    run<MaliciousRep3Share>(argc, argv);
}
