/*
 * parties/arith/mama-party.cpp
 *
 */

#include "protocols/spdz/mama_share.h"

#include "protocols/spdz/mama_prep.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "vm/field_machine.hpp"
#include "parties/shared/spdz.hpp"
#include "numeric/gfp.hpp"

template<int L, int N_MACS>
int run(OnlineMachine& machine)
{
    return machine.run<MamaShare<Gfp_<0, L>, N_MACS>, MamaShare<Gf2n, N_MACS>>();
}

int main(int argc, const char** argv)
{
    ez::EzOptionParser opt;
    OnlineOptions& online_options = OnlineOptions::singleton;
    online_options = {opt, argc, argv, MamaShare<Gfp_<0, 1>, 3>()};
    DishonestMajorityMachine machine(argc, argv, opt, online_options, 0);
    int length = min(online_options.prime_length() - 1, machine.get_lg2());
    int n_macs = DIV_CEIL(online_options.security_parameter, length);
    n_macs = 1 << int(ceil(log2(n_macs)));
    if (n_macs > 4)
        n_macs = 10;

    if (online_options.prime_limbs() == 1)
    {
#define X(N) if (n_macs == N) return run<1, N>(machine);
        X(1) X(2) X(4) X(10)
    }

    if (online_options.prime_limbs() == 2)
        return run<2, 1>(machine);

    if (online_options.prime_limbs() > 2)
        cerr << "Use MASCOT with large primes" << endl;
    else
        cerr << "Not compiled for choice of parameters" << endl;

    exit(1);
}
