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
    ThreadStatistics thread_statistics;
    TimerWithCommunication timer, online_timer, online_prep_timer;
    Timer inside_wait_timer;
    bool debug;

public:
    static thread_local ThreadQueue* thread_queue;

    map<string, TimerWithCommunication> timers;
    Timer wait_timer;
    NamedStatistics statistics;

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
    void finished(const ThreadJob& job, const ThreadStatistics& thread_statistics,
            const NamedStatistics& statistics = {});
    ThreadJob result();

    NamedCommunicationStatistics get_communication_statistics();

    void set_statistics(const ThreadStatistics& new_statistics);
    ThreadStatistics get_statistics();

    void start_timer();
    void stop_timer(Player& P);

    void start_online(Player& P, const TimerWithCommunication& prep_time);
    void stop_online(Player& P, const TimerWithCommunication& prep_time);
};
