/*
 * semi-bin-party.cpp
 *
 */

#include "boolean/share_party.h"
#include "boolean/semi_secret.h"

#include "boolean/share_party.hpp"
#include "boolean/share_secret.hpp"

#include "boolean/machine.hpp"
#include "boolean/program.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "boolean/processor.hpp"
#include "boolean/semi_secret.hpp"

#include "parties/shared/semi.hpp"

int main(int argc, const char** argv)
{
    GC::simple_binary_main<GC::SemiSecret>(argc, argv);
}
