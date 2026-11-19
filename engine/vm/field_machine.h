/*
 * vm/field_machine.h
 *
 */

#pragma once

#include "vm/ring_machine.h"
#include "vm/honest_majority_machine.h"
#include "support/ez_option_parser.h"
#include "numeric/gfp.h"
#include "protocols/spdz/mascot_prep.h"

#include "vm/online_options.hpp"

template<template<class T> class U, class V = HonestMajorityMachine>
class HonestMajorityFieldMachine
{
public:
    HonestMajorityFieldMachine(int argc, const char** argv);
    HonestMajorityFieldMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            int nplayers = 3);
};

template<template<class T> class U, template<class T> class V, class W, class X = Gf2n>
class FieldMachine
{
public:
    FieldMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            OnlineOptions& online_options, int nplayers = 0);
};

template<template<class U> class T, template<class U> class V = T,
        class W = Gf2n, class X = DishonestMajorityMachine>
class DishonestMajorityFieldMachine
{
public:
    DishonestMajorityFieldMachine(int argc, const char** argv,
            ez::EzOptionParser& opt, bool live_prep_default = true)
    {
        OnlineOptions& online_options = OnlineOptions::singleton;
        online_options = {opt, argc, argv, T<Gfp0>(), live_prep_default, W()};

        FieldMachine<T, V, X, W>(argc, argv, opt, online_options);
    }
};
