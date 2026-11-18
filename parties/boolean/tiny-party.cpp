/*
 * parties/boolean/tiny-party.cpp
 *
 */

#include "boolean/tiny_secret.h"
#include "boolean/tinier_secret.h"
#include "boolean/share_party.h"
#include "boolean/tiny_mc.h"
#include "boolean/vector_input.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/machine.hpp"
#include "boolean/processor.hpp"
#include "boolean/program.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/secret.hpp"
#include "boolean/tiny_prep.hpp"
#include "boolean/ccd_prep.hpp"
#include "boolean/tinier_share_prep.hpp"

#include "vm/instruction.hpp"
#include "protocols/common/mac_check.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "protocols/common/beaver.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "protocols/common/share.hpp"

int main(int argc, const char** argv)
{
    GC::simple_binary_main<GC::TinySecret<DEFAULT_SECURITY>>(argc, argv, 1000);
}
