/*
 * vm/thread_statistics.h
 *
 */

#pragma once

#include "net/player.h"
#include "support/execution_stats.h"

class ThreadStatistics
{
public:
    NamedCommunicationStatistics communication_statistics;
    ExecutionStats exe_statistics;

    ThreadStatistics() = default;

    ThreadStatistics(const NamedCommunicationStatistics& communication_statistics,
            const ExecutionStats& exe_statistics = {});

    ThreadStatistics& operator+=(const ThreadStatistics& other);

    ThreadStatistics operator-(const ThreadStatistics& other) const;
};
