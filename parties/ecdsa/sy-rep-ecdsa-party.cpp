/*
 * sy-ecdsa-party.cpp
 *
 */

#include "protocols/spdzwise/spdzwise_share.h"
#include "protocols/replicated/malicious_rep3_share.h"
#include "protocols/common/mac_check.h"
#include "protocols/spdzwise/spdzwise_mc.h"
#include "protocols/spdzwise/spdzwise_prep.h"
#include "protocols/spdzwise/spdzwise_input.h"
#include "numeric/gfp.h"
#include "numeric/gf2n.h"
#include "support/ez_option_parser.h"
#include "boolean/malicious_ccd_secret.h"
#include "boolean/semi_honest_rep_prep.h"

#include "vm/field_machine.hpp"
#include "protocols/replicated/replicated.hpp"
#include "protocols/common/share.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/spdzwise/spdzwise.hpp"
#include "protocols/spdzwise/spdzwise_prep.hpp"
#include "protocols/spdzwise/spdzwise_input.hpp"
#include "protocols/spdzwise/spdzwise_share.hpp"
#include "protocols/spdzwise/spdzwise_rep3_shuffler.hpp"
#include "vm/data_files.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/rep_prep.hpp"
#include "boolean/thread_master.hpp"
#include "numeric/gfp.hpp"
#include "parties/shared/malrep.hpp"

#include "ecdsa/hm_ecdsa_party.hpp"

template<class T>
using SpdzWiseRep3Share = SpdzWiseShare<MaliciousRep3Share<T>>;

int main(int argc, const char** argv)
{
    run<SpdzWiseRep3Share>(argc, argv);
}
