/*
 * rep4-ecsda-party.cpp
 *
 */

#include "parties/shared/rep4.hpp"

#include "ecdsa/hm_ecdsa_party.hpp"

int main(int argc, const char** argv)
{
    run<Rep4Share>(argc, argv);
}
