/*
 * support/memory_usage.h
 *
 */

#pragma once

#include <map>
using namespace std;

class MemoryUsage
{
    map<string, size_t> usage;

public:
    MemoryUsage& operator+=(const MemoryUsage& other)
    {
        for (auto& it : other.usage)
            usage[it.first] += it.second;
        return *this;
    }

    void update(const string& tag, size_t size)
    {
        usage[tag] = max(size, usage[tag]);
    }

    void add(const string& tag, size_t size)
    {
        usage[tag] += size;
    }

    size_t get(const string& tag)
    {
        return usage[tag];
    }

    void print()
    {
        for (auto& it : usage)
            cout << it.first << " required: " << 1e-9 * it.second << " (GB)" << endl;
    }
};
