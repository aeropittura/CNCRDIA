/*
 * vm/field_machine.hpp
 *
 */

#pragma once

#include "vm/field_machine.h"
#include "vm/honest_majority_machine.h"
#include "numeric/gfp.h"
#include "vm/online_machine.hpp"
#include "vm/online_options.hpp"

template<template<class U> class T, class V>
HonestMajorityFieldMachine<T, V>::HonestMajorityFieldMachine(int argc,
        const char **argv)
{
    ez::EzOptionParser opt;
    HonestMajorityFieldMachine<T>(argc, argv, opt);
}

template<template<class U> class T, class V>
HonestMajorityFieldMachine<T, V>::HonestMajorityFieldMachine(int argc,
        const char **argv, ez::EzOptionParser& opt, int nplayers)
{
    OnlineOptions online_options(opt, argc, argv, T<Gfp0>());
    FieldMachine<T, T, V>(argc, argv, opt, online_options,
            nplayers);
}

template<template<class U> class T, template<class U> class V, class W, class X>
FieldMachine<T, V, W, X>::FieldMachine(int argc, const char** argv,
        ez::EzOptionParser& opt, OnlineOptions& online_options, int nplayers)
{
    assert(nplayers or T<Gfpvar>::variable_players);
    W machine(argc, argv, opt, online_options, X(), nplayers);
    int n_limbs = online_options.prime_limbs();
    cerr << "Trying to run " << online_options.prime_length()
            << "-bit computation (" << n_limbs * 64 << "-bit representation)"
            << endl;
    switch (n_limbs)
    {
#undef X
#define X(L) \
    case L: \
        machine.template run<T<Gfp_<0, L>>, V<X>>(); \
        break;
#ifndef FEWER_PRIMES
    X(1) X(2) X(3) X(4)
#endif
#if GFP_MOD_SZ > 4 or defined(FEWER_PRIMES)
    X(GFP_MOD_SZ)
#endif
#undef X
    default:
        cerr << "Not compiled for " << online_options.prime_length() << "-bit primes" << endl;
        cerr << "Put 'MOD = -DGFP_MOD_SZ=" << n_limbs
                << "' in CONFIG.mine and run " << "'make " << argv[0] << "'"
                << endl;
        exit(1);
    }
}
