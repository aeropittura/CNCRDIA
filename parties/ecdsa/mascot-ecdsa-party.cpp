/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#define NO_MIXED_CIRCUITS

#define NO_SECURITY_CHECK

#include "boolean/tinier_secret.h"
#include "boolean/tiny_mc.h"
#include "boolean/vector_input.h"

#include "protocols/common/share.hpp"
#include "protocols/common/mac_check.hpp"
#include "boolean/secret.hpp"
#include "boolean/tinier_share_prep.hpp"
#include "ecdsa/ot_ecdsa_party.hpp"

#include <assert.h>

int main(int argc, const char** argv)
{
    run<Share>(argc, argv);
}
