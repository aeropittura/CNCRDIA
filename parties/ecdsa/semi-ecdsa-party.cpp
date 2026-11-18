/*
 * fake-spdz-ecdsa-party.cpp
 *
 */

#include "boolean/semi_secret.h"
#include "boolean/semi_prep.h"

#include "protocols/semi/hemi.hpp"
#include "protocols/semi/semi_mc.hpp"
#include "protocols/semi/semi_prep.hpp"
#include "protocols/semi/semi_input.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "boolean/semi_secret.hpp"
#include "ecdsa/ot_ecdsa_party.hpp"

#include <assert.h>

int main(int argc, const char** argv)
{
    run<SemiShare>(argc, argv);
}
