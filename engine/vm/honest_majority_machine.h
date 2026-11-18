/*
 * vm/honest_majority_machine.h
 *
 */

#pragma once

#include "vm/online_machine.h"

class HonestMajorityMachine : public OnlineMachine
{
public:
    HonestMajorityMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            OnlineOptions& online_opts, int n_players = 3);

    template<class V>
    HonestMajorityMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            OnlineOptions& online_opts, V, int nplayers) :
            HonestMajorityMachine(argc, argv, opt, online_opts, nplayers)
    {
    }
};
