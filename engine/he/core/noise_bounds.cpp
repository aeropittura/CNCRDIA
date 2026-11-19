/*
 * he/core/noise_bounds.cpp
 *
 */

#include "he/core/noise_bounds.h"
#include "he/offline/proof.h"
#include "protocols/spdz/cowgear_options.h"
#include <math.h>

SemiHomomorphicNoiseBounds::SemiHomomorphicNoiseBounds(const Bigint& p,
        int phi_m, int n, int sec, int slack_parameter, bool extra_h,
        const FheParameters& params) :
        p(p), phi_m(phi_m), n(n), sec(sec),
        slack(num_bits(Proof::slack(slack_parameter, sec, phi_m))),
        sigma(params.get_R())
{
    if (sigma <= 0)
        this->sigma = sigma = FheParameters().get_R();
    if (extra_h)
    {
        sigma *= 1.4;
        params.set_r(params.get_R() * 1.4);
    }

    if (OnlineOptions::singleton.has_option("verbose_he_setup"))
        cerr << "Standard deviation: " << this->sigma << endl;

    produce_epsilon_constants();

    // according to documentation of SCALE-MAMBA 1.7
    // excluding a factor of n because we don't always add up n ciphertexts
    assert(phi_m != 0);
    v_s = sigma * sqrt(phi_m);
    b_clean = (Bigint(phi_m) << (sec + 1)) * p
            * (20.5 + c1 * sigma * sqrt(phi_m) + 20 * c1 * v_s);
    // unify parameters by taking maximum over TopGear or not
    Bigint b_clean_top_gear = b_clean * 2;
    Bigint b_clean_not_top_gear = b_clean << max(slack - sec, 0);
    b_clean = max(b_clean_not_top_gear, b_clean_top_gear);
    b_scale = (c1 + c2 * v_s) * p * sqrt(phi_m / 12.0);
    int matrix_dimension = params.get_matrix_dimension();

    assert(matrix_dimension > 0);
    assert(params.secp() >= 0);
    drown = 1 + (p > 2 ? matrix_dimension : 1) * n * (Bigint(1) << params.secp());

    if (OnlineOptions::singleton.has_option("verbose_he_setup"))
    {
        cerr << "phi(m): " << phi_m << endl;
        cerr << "p * sqrt(phi(m) / 12): " << p * sqrt(phi_m / 12.0) << endl;
        cerr << "V_s: " << v_s << endl;
        cerr << "c1: " << c1 << endl;
        cerr << "c2: " << c2 << endl;
        cerr << "c1 + c2 * V_s: " << c1 + c2 * v_s << endl;
        cerr << "log(slack): " << slack << endl;
        cerr << "B_clean bits: " << b_clean.num_bits() << endl;
        cerr << "B_scale bits: " << b_scale.num_bits() << endl;
        cerr << "matrix dimension: " << matrix_dimension << endl;
        cerr << "drown sec: " << params.secp() << endl;
        cerr << "sec: " << sec << endl;
        cerr << "drown bits: " << drown.num_bits() << endl;
    }
}

Bigint SemiHomomorphicNoiseBounds::min_p0(const Bigint& p1)
{
    return p * drown * n * b_clean / p1 + b_scale;
}

Bigint SemiHomomorphicNoiseBounds::min_p0()
{
    // slack is already in b_clean
    return b_clean * drown * p;
}

double SemiHomomorphicNoiseBounds::min_phi_m(int log_q, double sigma)
{
    if (sigma <= 0)
        sigma = FheParameters().get_R();
    // the constant was updated using Martin Albrecht's LWE estimator in Mar 2022
    // found the following pairs for 128-bit security
    // and alpha = 0.7 * sqrt(2*pi) / q
    // m = 2048, log_2(q) = 68
    // m = 4096, log_2(q) = 138
    // m = 8192, log_2(q) = 302
    // m = 16384, log_2(q) = 560
    return 15.1 * log_q;
}

double SemiHomomorphicNoiseBounds::min_phi_m(int log_q, const FheParameters& params)
{
    return min_phi_m(log_q, params.get_R());
}

void SemiHomomorphicNoiseBounds::produce_epsilon_constants()
{
    double C[3];

    for (int i = 0; i < 3; i++)
    {
        C[i] = -1;
    }
    for (double x = 0.1; x < 10.0; x += .1)
    {
        double t = erfc(x), tp = 1;
        for (int i = 1; i < 3; i++)
        {
            tp *= t;
            double lgtp = log(tp) / log(2.0);
            if (C[i] < 0 && lgtp < -fhe_epsilon)
            {
                C[i] = pow(x, i);
            }
        }
    }

    c1 = C[1];
    c2 = C[2];
}

NoiseBounds::NoiseBounds(const Bigint& p, int phi_m, int n, int sec, int slack,
        const FheParameters& params) :
        SemiHomomorphicNoiseBounds(p, phi_m, n, sec, slack, sec, params)
{
    B_KS = p * c2 * this->sigma * phi_m / sqrt(12);
#ifdef NOISY
    cout << "p size: " << num_bits(p) << endl;
    cout << "phi(m): " << phi_m << endl;
    cout << "n: " << n << endl;
    cout << "sec: " << sec << endl;
    cout << "sigma: " << this->sigma << endl;
    cout << "B_clean size: " << num_bits(b_clean) << endl;
    cout << "B_scale size: " << num_bits(b_scale) << endl;
    cout << "B_KS size: " << num_bits(B_KS) << endl;
    cout << "drown size: " << num_bits(drown) << endl;
#endif
}

Bigint NoiseBounds::U1(const Bigint& p0, const Bigint& p1)
{
    Bigint tmp = n * b_clean / p1 + b_scale;
    return tmp * tmp + B_KS * p0 / p1 + b_scale;
}

Bigint NoiseBounds::U2(const Bigint& p0, const Bigint& p1)
{
    return U1(p0, p1) + n * b_clean / p1 + b_scale;
}

Bigint NoiseBounds::min_p0(const Bigint& p0, const Bigint& p1)
{
    return 2 * U2(p0, p1) * drown;
}

Bigint NoiseBounds::min_p0(const Bigint& p1)
{
    Bigint U = n * b_clean / p1 + 1 + b_scale;
    Bigint res = 2 * (U * U + U + b_scale) * drown;
    mpf_class div = (1 - 1. * min_p1() / p1);
    res = ceil(res / div);
#ifdef NOISY
    cout << "U size: " << num_bits(U) << endl;
    cout << "before div size: " << num_bits(res) << endl;
    cout << "div: " << div << endl;
    cout << "minimal p0 size: " << num_bits(res) << endl;
#endif
    return res;
}

Bigint NoiseBounds::min_p1()
{
    return max(Bigint(drown * B_KS), Bigint((phi_m * p) << 10));
}

Bigint NoiseBounds::option_p1()
{
    assert(b_scale != 0);
    // square equation parameters
    Bigint a, b, c;
    a = b_scale * b_scale + b_scale;
    b = -2 * a * min_p1();
    c = -n * b_clean * (2 * b_scale + 1) * min_p1() + n * n * b_scale * b_scale;
    // solve
    mpf_class s = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    Bigint res = ceil(s);
#ifdef VERBOSE
    cout << "Optimal p1 vs minimal: " << num_bits(res) << "/"
            << num_bits(min_p1()) << endl;
#endif
    return res;
}

double NoiseBounds::optimize(int& lg2p0, int& lg2p1)
{
    Bigint min_p1 = option_p1();
    Bigint min_p0 = this->min_p0(min_p1);
    while (this->min_p0(min_p0, min_p1) > min_p0)
      {
        min_p0 *= 2;
        min_p1 *= 2;
#ifdef VERBOSE
        cout << "increasing lengths: " << num_bits(min_p0) << "/"
            << num_bits(min_p1) << endl;
#endif
      }
    lg2p1 = num_bits(min_p1);
    lg2p0 = num_bits(min_p0);
    return min_phi_m(lg2p0 + lg2p1, sigma.get_d());
}
