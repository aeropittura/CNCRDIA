/*
 * OfflineMachineBase.h
 *
 */

#ifndef TOOLS_OFFLINEMACHINEBASE_H_
#define TOOLS_OFFLINEMACHINEBASE_H_

#include "support/ez_option_parser.h"
#include "net/server.h"
#include "net/player.h"

class OfflineParams
{
public:
    bool output;
    int nthreads;

    OfflineParams() : output(false), nthreads(0) {}
};

class OfflineMachineBase : virtual public OfflineParams
{
protected:
    ez::ezOptionParser opt;

public:
    Names N;
    int my_num, nplayers;
    long long ntriples, nTriplesPerThread;

    OfflineMachineBase();
    ~OfflineMachineBase();

    void parse_options(int argc, const char** argv);
    void start_networking_with_server(string hostname = "localhost", int portnum = 5000);
};

#endif /* TOOLS_OFFLINEMACHINEBASE_H_ */
