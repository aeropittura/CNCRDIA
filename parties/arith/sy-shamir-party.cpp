/*
 * sy-shamir-party.cpp
 *
 */

#include "protocols/spdzwise/spdzwise_share.h"
#include "protocols/shamir/malicious_shamir_share.h"
#include "protocols/spdzwise/spdzwise_mc.h"
#include "protocols/spdzwise/spdzwise_input.h"
#include "numeric/gfp.h"
#include "numeric/gf2n.h"
#include "boolean/ccd_secret.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/semi_honest_rep_prep.h"

#include "protocols/common/share.hpp"
#include "protocols/spdzwise/spdzwise.hpp"
#include "protocols/spdzwise/spdzwise_prep.hpp"
#include "protocols/spdzwise/spdzwise_input.hpp"
#include "protocols/spdzwise/spdzwise_share.hpp"
#include "parties/shared/shamir.hpp"
#include "parties/shared/malrep.hpp"

template<class T>
using SpdzWiseShamirShare = SpdzWiseShare<MaliciousShamirShare<T>>;

int main(int argc, const char** argv)
{
    ShamirMachineSpec<SpdzWiseShamirShare>(argc, argv);
}
