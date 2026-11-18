/*
 * boolean/access.h
 *
 */

#pragma once

#include "boolean/clear.h"

namespace GC
{

template <class T>
class ReadAccess
{
public:
    T& dest;
    const Clear address;
    const int length;

    ReadAccess(T& dest, Clear address, size_t length, size_t& complexity) :
        dest(dest), address(address), length(length)
    {
        complexity += length;
    }
};

template <class T>
class WriteAccess
{
public:
    const T& source;
    const Clear address;

    WriteAccess(Clear address, const T& source) :
        source(source), address(address) {}
};

struct ClearWriteAccess
{
	const Clear address, value;
	ClearWriteAccess(Clear address, Clear value) : address(address), value(value) {}
};

} /* namespace GC */
