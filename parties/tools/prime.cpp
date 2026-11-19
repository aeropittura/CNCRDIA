/*
 * parties/tools/prime.cpp
 *
 */

#include "numeric/gfp.h"
#include "numeric/setup.h"

int main(int argc, char** argv)
{
    int lgp = Gfp0::size_in_bits();
    if (argc > 1)
        lgp = atoi(argv[1]);
    if (argc > 2)
        cout << generate_prime(lgp, 1 << abs(atoi(argv[2])), atoi(argv[2]) <= 0) << endl;
    else
        cout << spdz_data_setup_primes(lgp) << endl;
}
