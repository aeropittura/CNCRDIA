/*
 * replicated-party.cpp
 *
 */

#include "boolean/share_party.h"
#include "boolean/semi_honest_rep_prep.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/machine.hpp"
#include "boolean/processor.hpp"
#include "boolean/program.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/rep_prep.hpp"

#include "vm/instruction.hpp"
#include "protocols/replicated/malicious_rep_mc.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/beaver.hpp"

int main(int argc, const char** argv)
{
    GC::simple_binary_main<GC::SemiHonestRepSecret>(argc, argv);
}
