/*
 * vm/honest_majority_machine.h
 *
 */

#pragma once

#include "vm/online_machine.h"

class HonestMajorityMachine : public OnlineMachine
{
public:
    HonestMajorityMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            OnlineOptions& online_options, int n_players = 3);

    template<class V>
    HonestMajorityMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            OnlineOptions& online_options, V, int nplayers) :
            HonestMajorityMachine(argc, argv, opt, online_options, nplayers)
    {
    }
};
