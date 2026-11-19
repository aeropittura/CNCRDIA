/*
 * he/core/generator.h
 *
 */

#pragma once

#include <vector>
using namespace std;

#include "numeric/bigint.h"
#include "numeric/modp.h"

template <class T>
class Generator
{
 public:
    virtual ~Generator() {}
    virtual Generator* clone() const = 0;
    virtual void get(T& x) const = 0;
};

template <class T>
class Iterator : public Generator<T>
{
    const vector<T>& v;
    mutable size_t i;

public:
    Iterator(const vector<T>& v) : v(v), i(0) {}
    Generator<T>* clone() const { return new Iterator(*this); }
    void get(T& x) const { x = v[i]; i++; }
};

class ConversionIterator : public Generator<Bigint>
{
    const vector<modp>& v;
    const ZpData& zp_d;
    mutable size_t i;

public:
    ConversionIterator(const vector<modp>& v, const ZpData& zp_d) : v(v), zp_d(zp_d), i(0) {}
    Generator<Bigint>* clone() const { return new ConversionIterator(*this); }
    void get(Bigint& x) const { to_bigint(x, v[i], zp_d); i++; }
};

class WriteConversionIterator : public Generator<Bigint>
{
    vector<modp>& v;
    const ZpData& zp_d;
    mutable size_t i;
    mutable Bigint tmp;

public:
    WriteConversionIterator(vector<modp>& v, const ZpData& zp_d) : v(v), zp_d(zp_d), i(0) {}
    Generator<Bigint>* clone() const { return new WriteConversionIterator(*this); }
    void get(Bigint& x) const { tmp = x; v[i].convert_destroy(tmp, zp_d); i++; }
};
