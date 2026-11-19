/*
 * parties/tools/hyper.cpp
 *
 */

#include "numeric/gfpvar.h"

#include "protocols/shamir/shamir.hpp"

int main(int argc, char** argv)
{
    assert(argc > 2);
    Gfpvar::initialize_field(argv[3]);
    vector<vector<Gfpvar>> hyper;
    int t = atoi(argv[1]);
    int n = atoi(argv[2]);
    Shamir<ShamirShare<Gfpvar>>::get_hyper(hyper, t, n);
    OctetStream os;
    os.store(hyper);
    ofstream out(Shamir<ShamirShare<Gfpvar>>::hyper_filename(t, n));
    os.output(out);
}
