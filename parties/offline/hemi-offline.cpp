
#include "boolean/tinier_secret.h"
#include "boolean/semi.h"
#include "boolean/semi_prep.h"
#include "protocols/semi/hemi_share.h"

#include "parties/shared/semi.hpp"
#include "numeric/gfp.hpp"
#include "vm/field_machine.hpp"
#include "vm/offline_machine.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/semi/hemi_prep.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    DishonestMajorityFieldMachine<HemiShare, HemiShare, Gf2nShort,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
