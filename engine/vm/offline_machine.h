/*
 * vm/offline_machine.h
 *
 */

#pragma once

#include "vm/online_machine.h"
#include "vm/data_files.h"
#include "vm/base_machine.h"
#include "net/crypto_player.h"

template<class W>
class OfflineMachine : public W, BaseMachine
{
    DataPositions usage;
    Names& player_names;
    Player& P;

    template<class T>
    void generate();

    int buffered_total(size_t required, size_t batch);

public:
    template<class V>
    OfflineMachine(int argc, const char** argv,
            ez::EzOptionParser& opt, OnlineOptions& online_options, V,
            int nplayers = 0);
    ~OfflineMachine();

    template<class T, class U>
    int run();

    const Names& get_N();
};
