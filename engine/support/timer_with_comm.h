/*
 * support/timer_with_comm.h
 *
 */

#pragma once

#include "support/time_func.h"
#include "net/player.h"
#include "vm/thread_stats.h"

class TimerWithComm : public Timer
{
    ThreadStats total_stats, last_stats;
    Timer cpu_timer;

public:
    TimerWithComm(const Timer& other = {});
    TimerWithComm(double time);

    void start(const ThreadStats& stats = {});
    void stop(const ThreadStats& stats = {});

    size_t bytes_sent() const;
    double mb_sent() const;
    size_t rounds() const;

    const ExecutionStats& exe_stats() const;

    TimerWithComm operator+(const TimerWithComm& other);
    TimerWithComm operator-(const TimerWithComm& other);
    TimerWithComm& operator+=(const TimerWithComm& other);
    TimerWithComm& operator-=(const TimerWithComm& other);

    string full();

    friend ostream& operator<<(ostream& os, const TimerWithComm& stats);
};
