/*
 * parties/arith/dealer-ring-party.cpp
 *
 */

#include "protocols/dealer/dealer_share.h"
#include "protocols/dealer/dealer_input.h"
#include "protocols/dealer/dealer.h"

#include "vm/ring_machine.hpp"
#include "vm/machine.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/dealer/dealer_prep.hpp"
#include "protocols/dealer/dealer_input.hpp"
#include "protocols/dealer/dealer_mc.hpp"
#include "protocols/dealer/dealer_matrix_prep.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/semi/semi_input.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/replicated_prep.hpp"
#include "protocols/replicated/malrep_ring_prep.hpp"
#include "protocols/semi/semi_mc.hpp"
#include "boolean/dealer_prep.h"
#include "boolean/semi_prep.h"
#include "boolean/semi_secret.hpp"
#include "parties/shared/semi.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<DealerRingShare, DealerShare>(argc, argv, 0);
}
