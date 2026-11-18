/*
 * parties/offline/simple-offline.cpp
 *
 */

#include "he/offline/simple_machine.h"
#include "support/callgrind.h"

int main(int argc, const char** argv)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    SimpleMachine machine(argc, argv);
    CALLGRIND_START_INSTRUMENTATION;
    machine.run();
}
