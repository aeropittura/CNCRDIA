/*
 * support/named_stats.h
 *
 */

#pragma once

#include <map>
#include <string>

using namespace std;

class NamedStats : public map<string, long>
{
public:
    NamedStats& operator+=(const NamedStats& other);

    void print();
};
