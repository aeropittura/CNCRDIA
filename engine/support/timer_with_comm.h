/*
 * support/timer_with_comm.h
 *
 */

#pragma once

#include "support/time_func.h"
#include "net/player.h"
#include "vm/thread_stats.h"

class TimerWithCommunication : public Timer
{
    ThreadStatistics total_statistics, last_statistics;
    Timer cpu_timer;

public:
    TimerWithCommunication(const Timer& other = {});
    TimerWithCommunication(double time);

    void start(const ThreadStatistics& statistics = {});
    void stop(const ThreadStatistics& statistics = {});

    size_t bytes_sent() const;
    double mb_sent() const;
    size_t rounds() const;

    const ExecutionStats& exe_statistics() const;

    TimerWithCommunication operator+(const TimerWithCommunication& other);
    TimerWithCommunication operator-(const TimerWithCommunication& other);
    TimerWithCommunication& operator+=(const TimerWithCommunication& other);
    TimerWithCommunication& operator-=(const TimerWithCommunication& other);

    string full();

    friend ostream& operator<<(ostream& os, const TimerWithCommunication& statistics);
};
