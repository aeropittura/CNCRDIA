/*
 * support/timer_with_comm.cpp
 *
 */

#include "support/timer_with_comm.h"
#include "vm/online_options.h"

TimerWithCommunication::TimerWithCommunication(const Timer& other) :
        Timer(other), cpu_timer(CLOCK_PROCESS_CPUTIME_ID)
{
}

TimerWithCommunication::TimerWithCommunication(double time) :
        TimerWithCommunication(Timer(time))
{
}

void TimerWithCommunication::start(const ThreadStatistics& statistics)
{
    Timer::start();
    last_statistics = statistics;
    cpu_timer.start();
}

void TimerWithCommunication::stop(const ThreadStatistics& statistics)
{
    Timer::stop();
    total_statistics += statistics - last_statistics;
    cpu_timer.stop();
}

size_t TimerWithCommunication::bytes_sent() const
{
    return total_statistics.communication_statistics.sent;
}

double TimerWithCommunication::mb_sent() const
{
    return total_statistics.communication_statistics.sent * 1e-6;
}

size_t TimerWithCommunication::rounds() const
{
    size_t res = 0;
    for (auto& x : total_statistics.communication_statistics)
        res += x.second.rounds;
    return res;
}

TimerWithCommunication TimerWithCommunication::operator +(const TimerWithCommunication& other)
{
    TimerWithCommunication res = *this;
    res += other;
    return res;
}

TimerWithCommunication TimerWithCommunication::operator -(const TimerWithCommunication& other)
{
    TimerWithCommunication res = *this;
    res.Timer::operator-=(other);
    res.total_statistics = total_statistics - other.total_statistics;
    return res;
}

TimerWithCommunication& TimerWithCommunication::operator +=(const TimerWithCommunication& other)
{
    Timer::operator+=(other);
    total_statistics += other.total_statistics;
    return *this;
}

TimerWithCommunication& TimerWithCommunication::operator -=(const TimerWithCommunication& other)
{
    *this = *this - other;
    return *this;
}

string TimerWithCommunication::full()
{
    stringstream tmp;
    tmp << elapsed() << " seconds";
    if (mb_sent() > 0)
        tmp << " (" << *this << ")";
    return tmp.str();
}

const ExecutionStats& TimerWithCommunication::exe_statistics() const
{
    return total_statistics.exe_statistics;
}

ostream& operator<<(ostream& os, const TimerWithCommunication& statistics)
{
    os << statistics.mb_sent() << " MB, " << statistics.rounds() << " rounds, "
            << statistics.cpu_timer.elapsed() << " CPU seconds, "
            << statistics.total_statistics.communication_statistics.total_time()
            << " communication seconds";
    if (OnlineOptions::singleton.has_option("ratio"))
        os << ", ratio " << (statistics.cpu_timer.elapsed() / statistics.elapsed());
    return os;
}
