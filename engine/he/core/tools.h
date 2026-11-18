/*
 * he/core/tools.h
 *
 */

#pragma once

#include <vector>
using namespace std;

template <class T>
T sum(const vector<T>& summands)
{
    T res = summands[0];
    for (size_t i = 1; i < summands.size(); i++)
        res += summands[i];
    return res;
}
