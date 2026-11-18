/*
 * garbled/yao/yao_garble_master.cpp
 *
 */

#include "garbled/yao/yao_garble_master.h"
#include "garbled/yao/yao_garbler.h"

#include "boolean/machine.hpp"
#include "boolean/program.hpp"
#include "boolean/processor.hpp"
#include "boolean/secret.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "vm/instruction.hpp"
#include "garbled/yao/yao_wire.hpp"

YaoGarbleMaster::YaoGarbleMaster(bool continuous, OnlineOptions& opts, int threshold) :
        super(opts), continuous(continuous), threshold(threshold)
{
    PRNG G;
    G.ReSeed();
    delta = G.get_doubleword();
    delta.set_signal(1);
}

GC::Thread<GC::Secret<YaoGarbleWire>>* YaoGarbleMaster::new_thread(int i)
{
    return new YaoGarbler(i, *this);
}
