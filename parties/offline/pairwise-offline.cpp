#include "he/offline/pairwise_machine.h"
#include "support/callgrind.h"

int main(int argc, const char** argv)
{
    CALLGRIND_STOP_INSTRUMENTATION;
    RealPairwiseMachine machine(argc, argv);
    CALLGRIND_START_INSTRUMENTATION;
    machine.run();
}
