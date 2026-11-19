
#include "boolean/tinier_secret.h"
#include "boolean/semi.h"
#include "boolean/semi_prep.h"
#include "protocols/semi/semi2k_share.h"
#include "protocols/semi/semi_prep2k.h"
#include "numeric/gf2n.h"

#include "parties/shared/semi.hpp"
#include "numeric/gfp.hpp"
#include "vm/ring_machine.hpp"
#include "vm/offline_machine.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/replicated/rep_ring_only_edabit_prep.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    DishonestMajorityRingMachine<Semi2kShare, SemiShare,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
