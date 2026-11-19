/*
 * numeric/setup.hpp
 *
 */

#pragma once

#include "numeric/gfp.h"

template<class T>
void generate_online_setup(string dirname, Bigint& p, int lgp)
{
    int idx, m;
    spdz_data_setup_primes(p, lgp, idx, m);
    T::initialize_field(p);
    T::write_setup(dirname);
}

template<class T = Gfp>
void read_setup(const string& directory_prefix, int lgp = -1)
{
    Bigint p;
    bool montgomery = true;

    string filename = directory_prefix + "Params-Data";

    if (directory_prefix.compare("") == 0)
        filename = string(PREP_DIR "Params-Data");

#ifdef DEBUG_FILES
  cerr << "loading params from: " << filename << endl;
#endif
    ifstream inpf(filename.c_str());
    inpf >> p;
    inpf >> montgomery;

    if (inpf.fail())
    {
        if (lgp > 0)
        {
            if (OnlineOptions::singleton.verbose)
                cerr << "No modulus found in " << filename << ", generating "
                        << lgp << "-bit prime" << endl;
            T::initialize_default(lgp);
        }
        else
            throw file_error(filename.c_str());
    }
    else
        T::initialize_field(p, montgomery);

    inpf.close();

    if (OnlineOptions::singleton.verbose)
        cerr << "Using prime modulus " << T::pr() << endl;
}
