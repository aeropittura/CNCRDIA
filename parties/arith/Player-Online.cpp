/*
 * parties/arith/Player-Online.cpp
 *
 */

#include "vm/config.h"
#include "protocols/common/share.h"
#include "boolean/tinier_secret.h"
#include "numeric/gfp.hpp"
#include "vm/field_machine.h"

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    DishonestMajorityFieldMachine<Share>(argc, argv, opt, false);
}
