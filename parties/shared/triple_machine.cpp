/*
 * parties/shared/triple_machine.cpp
 *
 */

#include "ot/triple_machine.h"
#include "ot/n_party_triple_generator.h"
#include "ot/ot_triple_setup.h"
#include "numeric/gf2n.h"
#include "numeric/setup.h"
#include "protocols/spdz2k/spdz2k_share.h"
#include "support/ez_option_parser.h"
#include "numeric/setup.h"
#include "protocols/fake/fake_stuff.h"
#include "numeric/bit_vec.h"
#include "boolean/tinier_secret.h"

#include "protocols/common/share.hpp"
#include "protocols/fake/fake_stuff.hpp"
#include "protocols/spdz/mascot_prep.hpp"
#include "numeric/z2k.hpp"
#include "numeric/gfp.hpp"
#include "ot/n_party_triple_generator.hpp"

#include <iostream>
#include <fstream>
using namespace std;

void* run_ngenerator_thread(void* ptr)
{
    ((GeneratorThread*)ptr)->generate();
    return 0;
}

TripleMachine::TripleMachine(int argc, const char** argv) :
        n_connections(1), player(0), bonding(0),
        network_options(opt, argc, argv, 2, true)
{
    opt.add(
        "1", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of loops (default: 1).", // Help description.
        "-l", // Flag token.
        "--nloops" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Generate MACs (implies -a).", // Help description.
        "-m", // Flag token.
        "--macs" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Amplify triples.", // Help description.
        "-a", // Flag token.
        "--amplify" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Check triples (implies -m; always done with SPDZ2k).", // Help description.
        "-c", // Flag token.
        "--check" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "GF(p) items for chosen prime", // Help description.
        "-P", // Flag token.
        "--prime-field" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Channel bonding", // Help description.
        "-b", // Flag token.
        "--bonding" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Generate bits", // Help description.
        "-B", // Flag token.
        "--bits" // Flag token.
    );
    opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "SPDZ2k parameter, e.g., 64", // Help description.
        "-Z", // Flag token.
        "--Z2k" // Flag token.
    );
    opt.add(
        "64", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "SPDZ2k security parameter (default: 64)", // Help description.
        "-S", // Flag token.
        "--security" // Flag token.
    );

    parse_options(argc, argv);

    opt.get("-l")->getInt(nloops);
    generate_bits = opt.get("-B")->isSet;
    check = opt.get("-c")->isSet || generate_bits;
    correlation_check = opt.get("-c")->isSet;
    generate_macs = opt.get("-m")->isSet || check;
    amplify = opt.get("-a")->isSet || generate_macs;
    if (opt.isSet("-P"))
    {
        string tmp;
        opt.get("-P")->getString(tmp);
        prime = tmp;
    }
    bonding = opt.get("-b")->isSet;
    opt.get("-Z")->getInt(z2k);
    check |= z2k;
    amplify &= not z2k;
    z2s = z2k;
    if (opt.isSet("-S"))
        opt.get("-S")->getInt(z2s);

    // doesn't work with Montgomery multiplication
    if (prime)
        Gfpvar1::initialize_field(prime, false);
    else
        Gfpvar1::initialize_default(128, false);
    Gf2nLong::initialize_field();
    Gf2nShort::initialize_field();
}

template<class T, class U>
GeneratorThread* TripleMachine::new_generator(OtTripleSetup& setup, int i,
        U& mac_key)
{
    if (i == 0)
    {
        mac_key = T::LivePrep::get_mac_key(*player);
        T::MacCheck::setup(*player);
    }

    if (output and i == 0)
    {
        prep_data_directory = get_prep_sub_directory<T>(PREP_DIR, nplayers);
        T::clear::write_setup(prep_data_directory);
        write_mac_key(prep_data_directory, my_num, nplayers, mac_key);
    }

    return new typename T::TripleGenerator(setup, N[i % n_connections], i,
            n_triples_per_thread, nloops, *this);
}

void TripleMachine::run()
{
    cout << "my_num: " << my_num << endl;
    network_options.start_networking(N[0], my_num);
    n_connections = 1;
    if (bonding)
    {
        if (network_options.ip_filename.empty())
        {
            cerr << "Bonding only works with --ip-file-name" << endl;
            exit(1);
        }
        N[1].init(my_num, network_options.portnum_base + 1000,
                network_options.ip_filename + "2", nplayers);
        n_connections = 2;
    }
    // do the base OTs
    PlainPlayer P(N[0], "base");
    OtTripleSetup setup(P, true);
    player = &P;

    vector<GeneratorThread*> generators(nthreads);
    vector<pthread_t> threads(nthreads);

    for (int i = 0; i < nthreads; i++)
    {
        if (prime)
            generators[i] = new_generator<Share<Gfpvar1>>(setup, i, mac_keyp);
        else if (z2k)
        {
            if (z2k == 32 and z2s == 32)
                generators[i] = new_generator<Spdz2kShare<32, 32>>(setup, i, mac_keyz);
            else if (z2k == 64 and z2s == 64)
                generators[i] = new_generator<Spdz2kShare<64, 64>>(setup, i, mac_keyz);
            else if (z2k == 64 and z2s == 48)
                generators[i] = new_generator<Spdz2kShare<64, 48>>(setup, i, mac_keyz);
            else if (z2k == 66 and z2s == 64)
                generators[i] = new_generator<Spdz2kShare<66, 64>>(setup, i, mac_keyz);
            else if (z2k == 66 and z2s == 48)
                generators[i] = new_generator<Spdz2kShare<66, 48>>(setup, i, mac_keyz);
#ifdef RING_SIZE
            else if (z2k == RING_SIZE and z2s == SPDZ2K_DEFAULT_SECURITY)
                generators[i] = new_generator<Spdz2kShare<RING_SIZE, SPDZ2K_DEFAULT_SECURITY>>(setup, i, mac_keyz);
#endif
            else
                throw runtime_error("not compiled for k=" + to_string(z2k) + " and s=" + to_string(z2s));
        }
        else
            generators[i] = new_generator<Share<Gf2n>>(setup, i, mac_key2);
    }
    ntriples = generators[0]->nTriples * nthreads;
    cout <<"Setup generators\n";
    for (int i = 0; i < nthreads; i++)
    {
        // lock before starting thread to avoid race condition
        generators[i]->lock();
        pthread_create(&threads[i], 0, run_ngenerator_thread, generators[i]);
    }

    // wait for initialization, then start clock and computation
    for (int i = 0; i < nthreads; i++)
        generators[i]->wait();
    cout << "Starting computation" << endl;
    gettimeofday(&start, 0);
    for (int i = 0; i < nthreads; i++)
    {
        generators[i]->signal();
        generators[i]->unlock();
    }

    // wait for threads to finish
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i],NULL);
        cout << "thread " << i+1 << " finished\n" << flush;
    }

    map<string,Timer>& timers = generators[0]->timers;
    for (map<string,Timer>::iterator it = timers.begin(); it != timers.end(); it++)
    {
        double sum = 0;
        for (size_t i = 0; i < generators.size(); i++)
            sum += generators[i]->timers[it->first].elapsed();
        cout << it->first << " on average took time "
                << sum / generators.size() << endl;
    }

    gettimeofday(&stop, 0);
    double time = timeval_diff_in_seconds(&start, &stop);
    cout << "Time: " << time << endl;
    cout << "Throughput: " << ntriples / time << endl;

    for (size_t i = 0; i < generators.size(); i++)
        delete generators[i];
}
