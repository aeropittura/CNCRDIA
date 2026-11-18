/*
 * parties/offline/ot-offline.cpp
 *
 */

#include "ot/n_party_triple_generator.h"
#include "ot/triple_machine.h"

int main(int argc, const char** argv)
{
    TripleMachine(argc, argv).run();
}
