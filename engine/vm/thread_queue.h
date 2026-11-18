/*
 * vm/thread_queue.h
 *
 */

#pragma once

#include "vm/thread_job.h"
#include "vm/thread_stats.h"
#include "support/named_stats.h"

class ThreadQueue
{
    WaitQueue<ThreadJob> in, out;
    Lock lock;
    int left;
    ThreadStats thread_stats;
    TimerWithComm timer, online_timer, online_prep_timer;
    Timer inside_wait_timer;
    bool debug;

public:
    static thread_local ThreadQueue* thread_queue;

    map<string, TimerWithComm> timers;
    Timer wait_timer;
    NamedStats stats;

    ThreadQueue() :
            left(0), debug(false)
    {
    }

    bool available()
    {
        return left == 0;
    }

    void schedule(const ThreadJob& job);
    ThreadJob next();
    void finished(const ThreadJob& job);
    void finished(const ThreadJob& job, const ThreadStats& thread_stats,
            const NamedStats& stats = {});
    ThreadJob result();

    NamedCommStats get_comm_stats();

    void set_stats(const ThreadStats& new_stats);
    ThreadStats get_stats();

    void start_timer();
    void stop_timer(Player& P);

    void start_online(Player& P, const TimerWithComm& prep_time);
    void stop_online(Player& P, const TimerWithComm& prep_time);
};
