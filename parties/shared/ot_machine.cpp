#include "net/player.h"
#include "ot/ot_extension.h"
#include "ot/ot_extension_with_matrix.h"
#include "support/exceptions.h"
#include "support/time_func.h"

#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <iostream>

#include <sys/time.h>

#include "parties/shared/output_check.h"
#include "parties/shared/ot_machine.h"

//#define BASE_OT_DEBUG

class OtThreadInfo
{
    public:
 
    int thread_num;
    bool stop;
    int other_player_num;
    OtExtensionWithMatrix* ot_ext;
    int n_ots, nbase;
    BitVector receiver_input;
    int nloops;
};

void* run_otext_thread(void* ptr)
{
    OtThreadInfo *tinfo = (OtThreadInfo*) ptr;

    //int num = tinfo->thread_num;
    //int other_player_num = tinfo->other_player_num;
    printf("\tI am in thread %d\n", tinfo->thread_num);
    tinfo->ot_ext->transfer(tinfo->n_ots, tinfo->receiver_input, tinfo->nloops);
    return NULL;
}

OtMachine::OtMachine(int argc, const char** argv)
{
    opt.add(
        "", // Default.
        1, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "This player's number, 0/1 (required).", // Help description.
        "-p", // Flag token.
        "--player" // Flag token.
    );

    opt.add(
        "5000", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Base port number (default: 5000).", // Help description.
        "-pn", // Flag token.
        "--portnum" // Flag token.
    );

    opt.add(
        "localhost", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Host name(s) that player 0 is running on (default: localhost). Split with commas.", // Help description.
        "-h", // Flag token.
        "--hostname" // Flag token.
    );

    opt.add(
        "1024",
        0,
        1,
        0,
        "Number of extended OTs to run (default: 1024).",
        "-n",
        "--nOTs"
    );

    opt.add(
        "128", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of base OTs to run (default: 128).", // Help description.
        "-b", // Flag token.
        "--nbase" // Flag token.
    );

    opt.add(
        "s",
        0,
        1,
        0,
        "Mode for OT. a (asymmetric) or s (symmetric, i.e. play both sender/receiver) (default: s).",
        "-m",
        "--mode"
    );
    opt.add(
        "1",
        0,
        1,
        0,
        "Number of threads (default: 1).",
        "-x",
        "--nthreads"
    );

    opt.add(
        "1",
        0,
        1,
        0,
        "Number of loops (default: 1).",
        "-l",
        "--nloops"
    );

    opt.add(
        "1",
        0,
        1,
        0,
        "Number of subloops (default: 1).",
        "-s",
        "--nsubloops"
    );

    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Run in passive security mode.", // Help description.
        "-pas", // Flag token.
        "--passive" // Flag token.
    );

    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Write results to files.", // Help description.
        "-o", // Flag token.
        "--output" // Flag token.
    );

    opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Real base OT.", // Help description.
        "-r", // Flag token.
        "--real" // Flag token.
    );

    opt.parse(argc, argv);

    string hostname, ot_mode, usage;
    passive = false;
    opt.get("-p")->getInt(my_num);
    opt.get("-pn")->getInt(portnum_base);
    opt.get("-h")->getString(hostname);
    opt.get("-n")->getLong(n_ots);
    opt.get("-m")->getString(ot_mode);
    opt.get("--nthreads")->getInt(nthreads);
    opt.get("--nloops")->getInt(nloops);
    opt.get("--nsubloops")->getInt(nsubloops);
    opt.get("--nbase")->getInt(nbase);
    if (opt.isSet("-pas"))
        passive = true;

    if (!opt.isSet("-p"))
    {
        opt.getUsage(usage);
        cout << usage;
        exit(0);
    }

    cout << "Player 0 host name = " << hostname << endl;
    cout << "Creating " << n_ots << " extended OTs in " << nthreads << " threads\n";
    cout << "Running in mode " << ot_mode << endl;

    if (passive)
        cout << "Running with PASSIVE security only\n";

    if (nbase < 128)
        cout << "WARNING: only using " << nbase << " seed OTs, using these for OT extensions is insecure.\n";

    if (ot_mode.compare("s") == 0)
        ot_role = BOTH;
    else if (ot_mode.compare("a") == 0)
    {
        if (my_num == 0)
            ot_role = SENDER;
        else
            ot_role = RECEIVER;
    }
    else
    {
        cerr << "Invalid OT mode argument: " << ot_mode << endl;
        exit(1);
    }

    // Several names for multiplexing
    unsigned int pos = 0;
    while (pos < hostname.length())
    {
        string::size_type new_position = hostname.find(',', pos);
        if (new_position == string::npos)
            new_position = hostname.length();
        int len = new_position - pos;
        string name = hostname.substr(pos, len);
        pos = new_position + 1;

        vector<string> names(2);
        names[my_num] = "localhost";
        names[1-my_num] = name;
        N.push_back(new Names(my_num, portnum_base + 1000 * N.size(), names));
    }

    P = new RealTwoPartyPlayer(*N[0], 1 - my_num, "machine");

    timeval baseOTstart, baseOTend;
    gettimeofday(&baseOTstart, NULL);
    // swap role for base OTs
    if (opt.isSet("-r"))
        bot_ = new BaseOT(nbase, P, INV_ROLE(ot_role));
    else
        bot_ = new FakeOt(nbase, P, INV_ROLE(ot_role));
    cout << "real mode " << opt.isSet("-r") << endl;
    BaseOT& bot = *bot_;
    bot.exec_base();
    gettimeofday(&baseOTend, NULL);
    double basetime = timeval_diff(&baseOTstart, &baseOTend);
    cout << "\t\tBaseTime (" << role_to_str(ot_role) << "): " << basetime/1000000 << endl << flush;

    // Receiver send something to force synchronization
    // (since Sender finishes base_ots before Receiver)
    int a = 3;
    vector<OctetStream> os(2);
    os[0].store(a);
    P->send_receive_player(os);
    os[1].get(a);
    cout << a << endl;

#ifdef BASE_OT_DEBUG
    // check base OTs
    bot.check();
    // check after extending with PRG a few times
    for (int i = 0; i < 8; i++)
    {
        bot.extend_length();
        bot.check();
    }
    cout << "Verifying base OTs (debugging)\n";
#endif

    // convert base_ot selection bits to BitVector
    // (not already BitVector due to legacy PVW code)
    base_receiver_input = bot.receiver_inputs;
    base_receiver_input.resize(nbase);
}

OtMachine::~OtMachine()
{
    for (auto names : N)
        delete names;
    delete bot_;
    delete P;
}


void OtMachine::run()
{
    // divide n_ots between threads and loops
    n_ots = DIV_CEIL(n_ots, nthreads * nloops);
    // round up to multiple of base OTs and subloops
    // discount for discarded OTs
    n_ots = DIV_CEIL(n_ots + 2 * 128, nbase * nsubloops) * nbase * nsubloops - 2 * 128;
    cout << "Running " << n_ots << " OT extensions per thread and loop\n" << flush;

    // PRG for generating inputs etc
    PRNG G;
    G.re_seed();
    BitVector receiver_input(n_ots);
    receiver_input.randomize(G);
    BaseOT& bot = *bot_;

    cout << "Initialize OT Extension\n";
    vector<OtThreadInfo> tinfos(nthreads);
    vector<pthread_t> threads(nthreads);
    timeval OTextstart, OTextend;
    gettimeofday(&OTextstart, NULL);

    // copy base inputs/outputs for each thread
    vector<BitVector> base_receiver_input_copy(nthreads);
    vector<vector< array<BitVector, 2> > > base_sender_inputs_copy(nthreads, vector<array<BitVector, 2> >(nbase));
    vector< vector<BitVector> > base_receiver_outputs_copy(nthreads, vector<BitVector>(nbase));
    vector<TwoPartyPlayer*> players(nthreads);

    for (int i = 0; i < nthreads; i++)
    {
        tinfos[i].receiver_input.assign(receiver_input);

        base_receiver_input_copy[i].assign(base_receiver_input);
        for (int j = 0; j < nbase; j++)
        {
            base_sender_inputs_copy[i][j][0].assign(bot.sender_inputs[j][0]);
            base_sender_inputs_copy[i][j][1].assign(bot.sender_inputs[j][1]);
            base_receiver_outputs_copy[i][j].assign(bot.receiver_outputs[j]);
        }
        // now setup resources for each thread
        // round robin with the names
        players[i] = new RealTwoPartyPlayer(*N[i % N.size()], 1 - my_num,
                "thread" + to_string(i));
        tinfos[i].thread_num = i+1;
        tinfos[i].other_player_num = 1 - my_num;
        tinfos[i].n_ots = n_ots;
        tinfos[i].ot_ext = new OtExtensionWithMatrix(
                players[i],
                ot_role,
                passive,
                nsubloops);
        tinfos[i].ot_ext->init(base_receiver_input_copy[i],
                base_sender_inputs_copy[i], base_receiver_outputs_copy[i]);
        tinfos[i].nloops = nloops;

        // create the thread
        pthread_create(&threads[i], NULL, run_otext_thread, &tinfos[i]);
        
        // extend base OTs with PRG for the next thread
        bot.extend_length();
    }
    // wait for threads to finish
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(threads[i],NULL);
        cout << "thread " << i+1 << " finished\n" << flush;
    }

    map<string,long long>& times = tinfos[0].ot_ext->times;
    for (map<string,long long>::iterator it = times.begin(); it != times.end(); it++)
    {
        long long sum = 0;
        for (int i = 0; i < nthreads; i++)
            sum += tinfos[i].ot_ext->times[it->first];

        cout << it->first << " on average took time "
                << double(sum) / nthreads / 1e6 << endl;
    }

    gettimeofday(&OTextend, NULL);
    double totaltime = timeval_diff(&OTextstart, &OTextend);
    cout << "Time for OTExt threads (" << role_to_str(ot_role) << "): " << totaltime/1000000 << endl << flush;

    if (opt.isSet("-o"))
    {
        BitVector receiver_output, sender_output;
        char filename[1024];
        snprintf(filename, 1024, RECEIVER_INPUT, my_num);
        ofstream outf(filename);
        receiver_input.output(outf, false);
        outf.close();
        snprintf(filename, 1024, RECEIVER_OUTPUT, my_num);
        outf.open(filename);
        for (unsigned int i = 0; i < n_ots; i++)
        {
            receiver_output.assign_bytes((char*) tinfos[0].ot_ext->get_receiver_output(i), sizeof(__m128i));
            receiver_output.output(outf, false);
        }
        outf.close();

        for (int i = 0; i < 2; i++)
        {
            snprintf(filename,1024, SENDER_OUTPUT, my_num, i);
            outf.open(filename);
            for (int j = 0; j < n_ots; j++)
            {
                sender_output.assign_bytes((char*) tinfos[0].ot_ext->get_sender_output(i, j), sizeof(__m128i));
                sender_output.output(outf, false);
            }
            outf.close();
        }
    }

    for (int i = 0; i < nthreads; i++)
    {
        delete players[i];
        delete tinfos[i].ot_ext;
    }
}
