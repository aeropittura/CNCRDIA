/*
 * support/execution_stats.h
 *
 */

#pragma once

#include <map>
using namespace std;

class ExecutionStats : public map<int, size_t>
{
public:
    ExecutionStats& operator+=(const ExecutionStats& other);

    ExecutionStats operator-(const ExecutionStats& other) const;;

    void print() const;
};
