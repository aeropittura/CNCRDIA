/*
 * parties/ecdsa/atlas-ecdsa-party.cpp
 *
 */

#define NO_MIXED_CIRCUITS

#include "parties/shared/atlas.hpp"

#include "ecdsa/hm_ecdsa_party.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    ShamirOptions(opt, argc, argv);
    run<AtlasShare>(argc, argv);
}
