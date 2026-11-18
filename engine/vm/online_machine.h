/*
 * vm/online_machine.h
 *
 */

#pragma once

#include "vm/online_options.h"
#include "numeric/gf2n.h"
#include "net/player.h"

class OnlineMachine
{
protected:
    int argc;
    const char** argv;
    OnlineOptions& online_opts;

    Names playerNames;

    bool use_encryption;

    ez::ezOptionParser& opt;

    int nplayers;

public:
    template<class V = gf2n>
    OnlineMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            OnlineOptions& online_opts, int nplayers = 0, V = {});

    void start_networking();

    template<class T, class U>
    int run();
    template<class T, class U>
    int run_with_error();

    Player* new_player(const string& id_base);

    int get_lg2()
    {
        return online_opts.lg2;
    }
};

class DishonestMajorityMachine : public OnlineMachine
{
public:
    template<class V>
    DishonestMajorityMachine(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, V,
            int nplayers = 0);
    DishonestMajorityMachine(int argc, const char** argv,
            ez::ezOptionParser& opt, OnlineOptions& online_opts, int nplayers);
};
