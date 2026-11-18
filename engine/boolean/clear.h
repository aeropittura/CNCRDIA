/*
 * boolean/clear.h
 *
 */

#pragma once

#include "numeric/integer.h"

namespace GC
{

class Clear : public Integer
{
public:
    static string type_string() { return "clear"; }

    Clear() : Integer() {}
    Clear(long a) : Integer(a) {}
    Clear(const IntBase& x) { IntBase::operator=(x); }

    void xor_(const Clear& x, const Clear& y) { a = x.a ^ y.a; }
    void xor_(const Clear& x, long y) { a = x.a ^ y; }
};

} /* namespace GC */
