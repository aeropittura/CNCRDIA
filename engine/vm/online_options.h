/*
 * vm/online_options.h
 *
 */

#pragma once

#include "support/ez_option_parser.h"
#include "numeric/bigint.h"
#include "numeric/setup.h"
#include "numeric/gf2n.h"

class OnlineOptions
{
    void finalize_with_error(ez::EzOptionParser& opt);

    bool have_warned_about_comp_sec;

public:
    static OnlineOptions singleton;

    bool interactive;
    int lgp;
    int lg2;
    Bigint prime;
    bool live_prep;
    int playerno;
    std::string progname;
    int batch_size;
    std::string memtype;
    bool bits_from_squares;
    bool direct;
    int bucket_size;
    int security_parameter;
    bool use_security_parameter;
    std::string command_private_input_file;
    std::string command_private_output_file;
    bool verbose;
    bool file_prep_per_thread;
    int trunc_error;
    int opening_sum, max_broadcast;
    bool receive_threads;
    std::string disk_memory;
    vector<long> args;
    vector<string> options;
    string executable;
    bool code_locations;
    bool semi_honest;

    OnlineOptions();
    OnlineOptions(ez::EzOptionParser& opt, int argc, const char** argv,
            bool security);
    OnlineOptions(ez::EzOptionParser& opt, int argc, const char** argv,
            int default_batch_size = 0, bool default_live_prep = true,
            bool variable_prime_length = false, bool security = true);
    template<class T, class V = Gf2n>
    OnlineOptions(ez::EzOptionParser& opt, int argc, const char** argv, T,
            bool default_live_prep = true, V = {});
    template<class T>
    OnlineOptions(T);
    ~OnlineOptions() {}

    void finalize(ez::EzOptionParser& opt, int argc, const char** argv,
            bool networking = true);

    void set_trunc_error(ez::EzOptionParser& opt);

    int prime_length();
    int prime_limbs();

    template<class T>
    string prep_directory_prefix(int nplayers)
    {
        int lgp = this->lgp;
        if (prime)
            lgp = num_bits(prime);
        return get_prep_sub_directory<T>(PREP_DIR, nplayers, lgp);
    }

    bool has_option(const string& option)
    {
        return find(options.begin(), options.end(), option) != options.end();
    }

    bool has_parameter(const string& param);
    int get_parameter(const string& param);

    int comp_sec();
};
