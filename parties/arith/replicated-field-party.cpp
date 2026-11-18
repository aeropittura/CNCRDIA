/*
 * replicated-field-party.cpp
 *
 */

#include "numeric/gfp.hpp"
#include "vm/field_machine.hpp"
#include "parties/shared/rep.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityFieldMachine<Rep3Share>(argc, argv);
}
