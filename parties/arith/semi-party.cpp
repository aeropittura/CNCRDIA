/*
 * semi-party.cpp
 *
 */

#include "numeric/gfp.h"
#include "protocols/semi/semi_share.h"
#include "support/switchable_output.h"
#include "boolean/semi_prep.h"

#include "vm/field_machine.hpp"
#include "parties/shared/semi.hpp"
#include "boolean/share_secret.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityFieldMachine<SemiShare>(argc, argv, opt);
}
