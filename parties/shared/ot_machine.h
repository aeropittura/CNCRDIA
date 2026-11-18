/*
 * parties/shared/ot_machine.h
 *
 */

#pragma once

#include "ot/ot_extension.h"
#include "support/ez_option_parser.h"

class OTMachine
{
    ez::ezOptionParser opt;
    OT_ROLE ot_role;

public:
    int my_num, portnum_base, nthreads, nloops, nsubloops, nbase;
    long nOTs;
    bool passive;
    TwoPartyPlayer* P;
    BitVector baseReceiverInput;
    BaseOT* bot_;
    vector<Names*> N;

    OTMachine(int argc, const char** argv);
    ~OTMachine();
    void run();
};
