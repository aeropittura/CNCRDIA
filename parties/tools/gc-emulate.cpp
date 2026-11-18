/*
 * gc-emulate.cpp
 *
 */

#include "boolean/program.h"
#include "boolean/instruction.h"
#include "boolean/fake_secret.h"
#include "boolean/machine.h"
#include "boolean/processor.h"

#include "boolean/processor.hpp"
#include "boolean/machine.hpp"
#include "boolean/program.hpp"
#include "boolean/thread.hpp"
#include "boolean/thread_master.hpp"
#include "vm/machine.hpp"
#include "vm/instruction.hpp"

int main(int argc, char** argv)
{
    if (argc < 2)
        exit(1);

    GC::Memory<GC::FakeSecret> dynamic_memory;
    GC::Machine<GC::FakeSecret> machine;
    GC::Processor<GC::FakeSecret> processor(machine);
    GC::Program program;
    program.parse(string(argv[1]) + "-0");
    machine.reset(program, dynamic_memory);
    processor.reset(program);
    if (argc > 2)
        processor.open_input_file(argv[2]);
    while (program.execute(processor, dynamic_memory) != GC::DONE_BREAK);
}
