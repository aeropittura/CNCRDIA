/*
 * numeric/value_interface.h
 *
 */

#pragma once

#include "support/exceptions.h"
#include "numeric/setup.h"

class OnlineOptions;
class Bigint;
class PRNG;

class ValueInterface
{
public:
    static const int MAX_EDABITS = 0;

    static const false_type characteristic_two;
    static const false_type prime_field;
    static const false_type invertible;
    static const false_type binary;
    static const false_type optimized_packing;
    static const true_type is_clear;

    typedef void Square;

    static int length() { return -1; }

    template<class T>
    static void init(bool mont = true) { (void) mont; }
    static void initialize_default(int, bool = true) {}
    static void initialize_field(const Bigint& = {}, bool = true) {}

    static void read_or_generate_setup(const string&, const OnlineOptions&) {}
    template<class T>
    static void generate_setup(string, int, int) {}
    template<class T>
    static void write_setup(int nplayers) { get_prep_sub_directory<T>(nplayers, true); }
    static void write_setup(string) {}
    static void check_setup(const string& directory);
    static const char* fake_options() { return ""; }

    static Bigint pr() { throw runtime_error("no prime modulus"); }

    void normalize() {}

    void randomize_part(PRNG&, int) { throw NotImplemented(); }
};
