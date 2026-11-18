
#include "numeric/gfp.hpp"
#include "boolean/tinier_secret.h"
#include "vm/field_machine.h"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    DishonestMajorityFieldMachine<Share>(argc, argv, opt);
}
