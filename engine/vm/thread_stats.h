/*
 * vm/thread_stats.h
 *
 */

#pragma once

#include "net/player.h"
#include "support/execution_stats.h"

class ThreadStats
{
public:
    NamedCommStats comm_stats;
    ExecutionStats exe_stats;

    ThreadStats() = default;

    ThreadStats(const NamedCommStats& comm_stats,
            const ExecutionStats& exe_stats = {});

    ThreadStats& operator+=(const ThreadStats& other);

    ThreadStats operator-(const ThreadStats& other) const;
};
