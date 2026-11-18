/*
 * mal-shamir-offline.cpp
 *
 */

#include "parties/shared/shamir.hpp"
#include "parties/shared/malrep.hpp"
#include "vm/offline_machine.hpp"

int main(int argc, const char** argv)
{
    auto& opts = ShamirOptions::singleton;
    ez::ezOptionParser opt;
    opts = {opt, argc, argv};
    HonestMajorityFieldMachine<MaliciousShamirShare,
            OfflineMachine<HonestMajorityMachine>>(argc, argv, opt,
            opts.nparties);
}
