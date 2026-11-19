/*
 * vm/thread_statistics.cpp
 *
 */

#include "vm/thread_stats.h"

ThreadStatistics::ThreadStatistics(const NamedCommunicationStatistics& communication_statistics,
        const ExecutionStats& exe_statistics) :
        communication_statistics(communication_statistics), exe_statistics(exe_statistics)
{
}

ThreadStatistics& ThreadStatistics::operator +=(const ThreadStatistics& other)
{
    communication_statistics += other.communication_statistics;
    exe_statistics += other.exe_statistics;
    return *this;
}

ThreadStatistics ThreadStatistics::operator -(const ThreadStatistics& other) const
{
    ThreadStatistics res;
    res.communication_statistics = communication_statistics - other.communication_statistics;
    res.exe_statistics = exe_statistics - other.exe_statistics;
    return res;
}
