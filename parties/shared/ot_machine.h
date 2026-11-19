/*
 * parties/shared/ot_machine.h
 *
 */

#pragma once

#include "ot/ot_extension.h"
#include "support/ez_option_parser.h"

class OtMachine
{
    ez::EzOptionParser opt;
    OT_ROLE ot_role;

public:
    int my_num, portnum_base, nthreads, nloops, nsubloops, nbase;
    long n_ots;
    bool passive;
    TwoPartyPlayer* P;
    BitVector base_receiver_input;
    BaseOT* bot_;
    vector<Names*> N;

    OtMachine(int argc, const char** argv);
    ~OtMachine();
    void run();
};
