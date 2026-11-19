/*
 * he/core/noise_bounds.h
 *
 */

#pragma once

#include "numeric/bigint.h"

int phi_n(int N);
class FheParameters;

class SemiHomomorphicNoiseBounds
{
protected:
    static const int fhe_epsilon = 55;

    const Bigint p;
    const int phi_m;
    const int n;
    const int sec;
    int slack;
    mpf_class sigma;

    Bigint b_clean;
    Bigint b_scale;
    Bigint drown;

    mpf_class c1, c2;
    mpf_class v_s;

    void produce_epsilon_constants();

public:
    SemiHomomorphicNoiseBounds(const Bigint& p, int phi_m, int n, int sec,
            int slack, bool extra_h, const FheParameters& params);
    // with scaling
    Bigint min_p0(const Bigint& p1);
    // without scaling
    Bigint min_p0();
    Bigint min_p0(bool scale, const Bigint& p1) { return scale ? min_p0(p1) : min_p0(); }
    static double min_phi_m(int log_q, double sigma);
    static double min_phi_m(int log_q, const FheParameters& params);

    Bigint get_b_clean() { return b_clean; }
};

// as per ePrint 2012:642 for slack = 0
class NoiseBounds : public SemiHomomorphicNoiseBounds
{
    Bigint B_KS;

public:
    NoiseBounds(const Bigint& p, int phi_m, int n, int sec, int slack,
            const FheParameters& params);
    Bigint U1(const Bigint& p0, const Bigint& p1);
    Bigint U2(const Bigint& p0, const Bigint& p1);
    Bigint min_p0(const Bigint& p0, const Bigint& p1);
    Bigint min_p0(const Bigint& p1);
    Bigint min_p1();
    Bigint option_p1();
    Bigint option_p0() { return min_p0(option_p1()); }
    double optimize(int& lg2p0, int& lg2p1);
};
