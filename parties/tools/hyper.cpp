/*
 * hyper.cpp
 *
 */

#include "numeric/gfpvar.h"

#include "protocols/shamir/shamir.hpp"

int main(int argc, char** argv)
{
    assert(argc > 2);
    gfpvar::init_field(argv[3]);
    vector<vector<gfpvar>> hyper;
    int t = atoi(argv[1]);
    int n = atoi(argv[2]);
    Shamir<ShamirShare<gfpvar>>::get_hyper(hyper, t, n);
    octetStream os;
    os.store(hyper);
    ofstream out(Shamir<ShamirShare<gfpvar>>::hyper_filename(t, n));
    os.output(out);
}
