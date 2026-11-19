/*
 * support/offline_machine_base.cpp
 *
 */

#include "support/offline_machine_base.h"
#include "support/int.h"

#include <string>
using namespace std;


OfflineMachineBase::OfflineMachineBase() :
        my_num(0), nplayers(0), ntriples(0),
        n_triples_per_thread(0)
{
}

OfflineMachineBase::~OfflineMachineBase()
{
}

void OfflineMachineBase::parse_options(int argc, const char** argv)
{
    opt.add(
        "2", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of parties (default: 2).", // Help description.
        "-N", // Flag token.
        "--nparties" // Flag token.
    );
    opt.add(
        "", // Default.
        1, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "This player's number, 0/1 (required).", // Help description.
        "-p", // Flag token.
        "--player" // Flag token.
    );
    opt.add(
        "1",
        0,
        1,
        0,
        "Number of threads (default: 1).",
        "-x",
        "--nthreads"
    );
    opt.add(
        "1000", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of triples (default: 1000).", // Help description.
        "-n", // Flag token.
        "--ntriples" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Write results to files.", // Help description.
        "-o", // Flag token.
        "--output" // Flag token.
    );

    opt.parse(argc, argv);
    if (!opt.isSet("-p"))
    {
        string usage;
        opt.getUsage(usage);
        cout << usage;
        exit(0);
    }

    opt.get("-p")->getInt(my_num);
    opt.get("-N")->getInt(nplayers);
    opt.get("-x")->getInt(nthreads);
    opt.get("-n")->get_long_long(ntriples);
    output = opt.get("-o")->isSet;

    n_triples_per_thread =  DIV_CEIL(ntriples, nthreads);
}

void OfflineMachineBase::start_networking_with_server(string hostname,
        int portnum)
{
    Server::start_networking(N, my_num, nplayers, hostname, portnum);
}
