/*
 * protocols/shamir/shamir_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"

class ShamirOptions
{
public:
    static ShamirOptions singleton;
    static ShamirOptions& s();

    int nparties;
    int threshold;

    ShamirOptions(int nparties = 3, int threshold = 1);
    ShamirOptions(ez::EzOptionParser& opt, int argc, const char** argv);

    void set_threshold(ez::EzOptionParser& opt);
};

class ShamirMachine : public ShamirOptions
{
};

template<template<class U> class T>
class ShamirMachineSpec
{
public:
    ShamirMachineSpec(int argc, const char** argv);
};
