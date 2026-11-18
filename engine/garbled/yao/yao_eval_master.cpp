/*
 * garbled/yao/yao_eval_master.cpp
 *
 */

#include "garbled/yao/yao_eval_master.h"
#include "garbled/yao/yao_evaluator.h"

#include "boolean/machine.hpp"
#include "boolean/program.hpp"
#include "boolean/processor.hpp"
#include "boolean/secret.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "vm/instruction.hpp"
#include "garbled/yao/yao_wire.hpp"

YaoEvalMaster::YaoEvalMaster(bool continuous, OnlineOptions& opts) :
        ThreadMaster<GC::Secret<YaoEvalWire>>(opts), continuous(continuous)
{
}

GC::Thread<GC::Secret<YaoEvalWire>>* YaoEvalMaster::new_thread(int i)
{
    return new YaoEvaluator(i, *this);
}
