
#include "boolean/tinier_secret.h"
#include "boolean/semi.h"
#include "boolean/semi_prep.h"

#include "parties/shared/semi.hpp"
#include "numeric/gfp.hpp"
#include "vm/field_machine.hpp"
#include "vm/offline_machine.hpp"
#include "protocols/spdz/mascot_prep.hpp"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    DishonestMajorityFieldMachine<SemiShare, SemiShare, Gf2n,
            OfflineMachine<DishonestMajorityMachine>>(argc, argv, opt);
}
