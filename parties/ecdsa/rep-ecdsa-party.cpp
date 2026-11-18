/*
 * mal-rep-ecdsa-party.cpp
 *
 */

#include "protocols/replicated/rep3_share.h"

#include "ecdsa/hm_ecdsa_party.hpp"

int main(int argc, const char** argv)
{
    run<Rep3Share>(argc, argv);
}
