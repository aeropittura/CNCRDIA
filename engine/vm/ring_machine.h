/*
 * vm/ring_machine.h
 *
 */

#pragma once

#include <string>
using namespace std;

#include "support/ez_option_parser.h"
#include "vm/online_options.h"
#include "net/player.h"
#include "vm/online_machine.h"

template<template<int L> class U, template<class T> class V>
class HonestMajorityRingMachine
{
public:
    HonestMajorityRingMachine(int argc, const char** argv, int nplayers = 3);
    HonestMajorityRingMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            int nplayers = 3);
};

template<template<int L> class U, template<class T> class V, class W>
class RingMachine
{
public:
    RingMachine(int argc, const char** argv, ez::EzOptionParser& opt,
            OnlineOptions& online_options, int nplayers = 0);
};

template<template<int K, int S> class U, template<class T> class V>
class HonestMajorityRingMachineWithSecurity
{
public:
    HonestMajorityRingMachineWithSecurity(int argc, const char** argv,
            ez::EzOptionParser& opt);
};

template<template<int K> class T, template<class V> class U,
        class W = DishonestMajorityMachine>
class DishonestMajorityRingMachine
{
public:
    DishonestMajorityRingMachine(int argc, const char** argv,
            ez::EzOptionParser& opt, bool live_prep_default = true)
    {
        OnlineOptions& online_options = OnlineOptions::singleton;
        online_options = {opt, argc, argv, T<64>(), live_prep_default};

        RingMachine<T, U, W>(argc, argv, opt, online_options);
    }
};
