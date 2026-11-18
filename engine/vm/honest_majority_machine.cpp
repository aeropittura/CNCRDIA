/*
 * vm/honest_majority_machine.cpp
 *
 */

#include "vm/honest_majority_machine.h"

#include "support/ez_option_parser.h"
#include "support/benchmarking.h"
#include "support/network_options.h"
#include "net/server.h"
#include "protocols/replicated/rep3_share.h"
#include "vm/machine.h"

#include "vm/online_machine.hpp"

HonestMajorityMachine::HonestMajorityMachine(int argc, const char** argv,
        ez::ezOptionParser& opt, OnlineOptions& online_opts, int nplayers) :
        OnlineMachine(argc, argv, opt, online_opts, nplayers)
{
    opt.add(
            "", // Default.
            0, // Required?
            0, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Unencrypted communication.", // Help description.
            "-u", // Flag token.
            "--unencrypted" // Flag token.
    );
    online_opts.finalize(opt, argc, argv);
    OnlineOptions::singleton = online_opts;

    use_encryption = not opt.get("-u")->isSet;

    if (not use_encryption)
        insecure("unencrypted communication");

    start_networking();
}
