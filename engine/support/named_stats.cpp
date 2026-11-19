/*
 * support/named_stats.cpp
 *
 */

#include "support/named_stats.h"

#include <iomanip>
#include <iostream>

NamedStatistics& NamedStatistics::operator+=(const NamedStatistics& other)
{
    for (auto x : other)
        (*this)[x.first] += x.second;
    return *this;
}

void NamedStatistics::print()
{
    long sum = 0;
    for (auto x : *this)
        sum += x.second;
    if (sum > 0)
        cerr << "Detailed costs:" << endl;
    for (auto x : *this)
    {
        if (x.second > 0)
        {
            cerr.fill(' ');
            cerr << "    ";
            cerr << setw(10) << x.second << " " << x.first << endl;
        }
    }
}
