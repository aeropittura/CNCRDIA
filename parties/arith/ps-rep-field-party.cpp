/*
 * parties/arith/ps-rep-field-party.cpp
 *
 */

#include "protocols/replicated/postsacri_rep_field_share.h"
#include "vm/field_machine.hpp"
#include "parties/shared/rep.hpp"
#include "parties/shared/malrep.hpp"
#include "protocols/replicated/post_sacrifice.hpp"
#include "numeric/gfp.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityFieldMachine<PostSacriRepFieldShare>(argc, argv);
}
