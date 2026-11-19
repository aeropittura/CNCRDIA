/*
 * numeric/setup.h
 *
 */

#pragma once

#include "numeric/bigint.h"
#include "support/mkpath.h"

#include <iostream>
#include <fstream>
using namespace std;

#ifndef PREP_DIR
#define PREP_DIR "Player-Data/"
#endif

/*
 * Routines to create and read setup files for the finite fields
 */

// Create setup file for gfp and Gf2n
template<class T>
void generate_prime_setup(string dir, int lgp);
template<class T>
void generate_online_setup(string dirname, Bigint& p, int lgp);

// Setup primes only
// Chooses a p of at least lgp bits
Bigint spdz_data_setup_primes(int lgp);
void spdz_data_setup_primes(Bigint& p,int lgp,int& idx,int& m);
void generate_prime(Bigint& p, int lgp, int m, bool force_degree = false);
Bigint generate_prime(int lgp, int m, bool force_degree = false);
int default_m(int& lgp, int& idx);

string get_prep_sub_directory(const string& prep_directory, int nparties, int log2mod,
        const string& type_short, bool create = false);

template<class T>
string get_prep_sub_directory(const string& prep_directory, int nparties, int log2mod,
        bool create = false)
{
    if (T::clear::length() > 1)
        log2mod = T::clear::length();
    return get_prep_sub_directory(prep_directory, nparties, log2mod, T::type_short(), create);
}

template<class T>
string get_prep_sub_directory(const string& prep_directory, int nparties, bool create =
        false)
{
    return get_prep_sub_directory<T>(prep_directory, nparties, T::clear::length(), create);
}

template<class T>
string get_prep_sub_directory(int nparties, bool create = false)
{
    return get_prep_sub_directory<T>(PREP_DIR, nparties, create);
}

template<class T>
void generate_prime_setup(string dir, int nparties, int lgp)
{
    Bigint p;
    generate_online_setup<typename T::clear>(
            get_prep_sub_directory<T>(dir, nparties, lgp), p, lgp);
}
