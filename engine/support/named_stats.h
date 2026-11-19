/*
 * support/named_stats.h
 *
 */

#pragma once

#include <map>
#include <string>

using namespace std;

class NamedStatistics : public map<string, long>
{
public:
    NamedStatistics& operator+=(const NamedStatistics& other);

    void print();
};
