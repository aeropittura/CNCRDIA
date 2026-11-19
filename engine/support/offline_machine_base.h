/*
 * support/offline_machine_base.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"
#include "net/server.h"
#include "net/player.h"

class OfflineParameters
{
public:
    bool output;
    int nthreads;

    OfflineParameters() : output(false), nthreads(0) {}
};

class OfflineMachineBase : virtual public OfflineParameters
{
protected:
    ez::EzOptionParser opt;

public:
    Names N;
    int my_num, nplayers;
    long long ntriples, n_triples_per_thread;

    OfflineMachineBase();
    ~OfflineMachineBase();

    void parse_options(int argc, const char** argv);
    void start_networking_with_server(string hostname = "localhost", int portnum = 5000);
};
