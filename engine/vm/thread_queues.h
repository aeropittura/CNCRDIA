/*
 * vm/thread_queues.h
 *
 */

#pragma once

#include "support/wait_queue.h"
#include "vm/thread_job.h"
#include "vm/thread_queue.h"

#include <barrier>

class ThreadQueues :
        public vector<ThreadQueue*>
{
    vector<int> available;

    barrier<>* sync_point;

    size_t n_tape_threads;

    size_t capacity_per_tape_thread();

    int get_n_per_thread(int n_items, vector<int>& to_use, int granularity = 1);
    int get_n_threads(int n_items, vector<int>& to_use, int base = 0,
            int granularity = 1);

    int distribute_no_setup(ThreadJob job, int n_items, int base,
            int granularity, const vector<void*>* supplies, vector<int>& to_use);

public:
    thread_local static int counter;

    ThreadQueues();

    void set_n_tape_threads(size_t n_threads);
    size_t get_n_tape_threads();

    int find_available();
    int get_n_per_thread(int n_items, int granularity = 1);
    // expects that the last slice is done by the caller
    int distribute(ThreadJob job, int n_items, int base = 0,
            int granularity = 1);
    pair<int, vector<int>> distribute_multi(ThreadJob job, int n_items,
            int base = 0, int granularity = 1);
    int distribute_no_setup(ThreadJob job, int n_items, int base = 0,
            int granularity = 1, const vector<void*>* supplies = 0);
    int distribute_with_sync(ThreadJob job, int n_items);
    void sync();
    void wrap_up(ThreadJob job);
    void wrap_up(ThreadJob job, vector<int>& to_use);

    TimerWithCommunication sum(const string& phase);

    void print_breakdown();

    ThreadStatistics total_statistics();
    NamedCommunicationStatistics max_communication();
};
