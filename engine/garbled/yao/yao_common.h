/*
 * garbled/yao/yao_common.h
 *
 */

#pragma once

#include <stdexcept>
#include <math.h>
#include <vector>
#include <array>

#include "support/exceptions.h"
#include "boolean/runtime_branching.h"
#include "boolean/thread_master.h"
#include "garbled/yao/yao_and_job.h"

#include <thread>

template<class T>
class YaoCommon : public GC::RuntimeBranching
{
    int log_n_threads;

    GC::ThreadMaster<GC::Secret<T>>& master;

public:
    static const int DONE = -1;
    static const int MORE = -2;

    long counter;

    vector<YaoAndJob<T>*> jobs;

    YaoCommon(GC::ThreadMaster<GC::Secret<T>>& master) :
        log_n_threads(8), master(master), counter(0)
    {
    }

    ~YaoCommon()
    {
        for (auto& job : jobs)
            delete job;
    }

    void init(typename T::Party& party)
    {
        jobs.resize(get_n_worker_threads());
        for (auto& job : jobs)
            job = new YaoAndJob<T>(party);
    }

    void set_n_program_threads(int n_threads)
    {
        log_n_threads = floor(log2(n_threads));
    }

    long gate_id(long thread_num)
    {
        return counter + (thread_num << (64 - log_n_threads));
    }

    int get_n_worker_threads()
    {
        return max(1u, thread::hardware_concurrency() / master.machine.nthreads);
    }

    vector<array<size_t, 2>> get_splits(const ArgVector& args, int threshold,
            int total);

    void wait(int n_threads)
    {
        for (int i = 0; i < n_threads; i++)
            jobs[i]->worker.done();
    }
};
