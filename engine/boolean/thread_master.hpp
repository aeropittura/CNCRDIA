/*
 * boolean/thread_master.hpp
 *
 */

#pragma once

#include "boolean/thread_master.h"
#include "boolean/program.h"

#include "boolean/instructions.h"

#include "support/benchmarking.h"

#include "boolean/machine.hpp"

namespace GC
{

template<class T>
ThreadMaster<T>* ThreadMaster<T>::singleton = 0;

template<class T>
ThreadMaster<T>& ThreadMaster<T>::s()
{
    if (singleton)
        return *singleton;
    else
        throw NoSingleton("no singleton, maybe threads not supported");
}

template<class T>
ThreadMaster<T>::ThreadMaster(OnlineOptions& opts) :
        P(0), opts(opts)
{
    if (singleton)
        throw runtime_error("there can only be one");
    singleton = this;
}

template<class T>
void ThreadMaster<T>::run_tape(int thread_number, int tape_number, int arg)
{
    threads.at(thread_number)->queue.schedule({tape_number, arg});
}

template<class T>
void ThreadMaster<T>::join_tape(int thread_number)
{
    threads.at(thread_number)->join_tape();
}

template<class T>
Thread<T>* ThreadMaster<T>::new_thread(int i)
{
    return new Thread<T>(i, *this);
}

template<class T>
void ThreadMaster<T>::run()
{
    if (opts.has_option("throw_exceptions"))
        run_with_error();
    else
    {
        try
        {
            run_with_error();
        }
        catch (exception& e)
        {
            cerr << "Fatal error: " << e.what() << endl;
            exit(1);
        }
    }
}

template<class T>
void ThreadMaster<T>::run_with_error()
{
    if (not opts.live_prep)
    {
        insecure("preprocessing from file in binary virtual machines");
    }

    P = new PlainPlayer(N, "main");

    machine.load_schedule(progname);
    machine.reset(machine.progs[0], memory);

    for (int i = 0; i < machine.nthreads; i++)
    {
        threads.push_back(new_thread(i));
        machine.queues.push_back(&threads.back()->queue);
    }

    // must start after constructor due to virtual functions
    for (auto thread : threads)
        thread->start();
    for (auto thread : threads)
        thread->join_tape();

    machine.reset_timer();

    machine.queues.at(0)->schedule(0);

    for (auto thread : threads)
        thread->finish();

    // synchronize
    vector<OctetStream> os(P->num_players());
    P->broadcast_receive(os);

    post_run();

    if (opts.verbose)
        machine.queues.print_breakdown();

    NamedCommunicationStatistics statistics = P->total_communication();
    ExecutionStats exe_statistics;
    for (auto thread : threads)
    {
        statistics += thread->P->total_communication();
        statistics += thread->extra_communication();
        exe_statistics += thread->processor.statistics;
        delete thread;
    }

    machine.queues.clear();

    if (not exe_statistics.empty() and opts.verbose)
        exe_statistics.print();

    if (opts.verbose)
        statistics.print();

    machine.print_timers();

    machine.print_communication(*P, statistics);

    delete P;
}

} /* namespace GC */
