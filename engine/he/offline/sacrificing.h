/*
 * he/offline/sacrificing.h
 *
 */

#pragma once

#include "net/player.h"
#include "protocols/common/mac_check.h"
#include "numeric/setup.h"
#include "numeric/gfpvar.h"

template <class T>
class TripleSacriFactory
{
public:
    vector<array<T, 3>> triples;

    virtual ~TripleSacriFactory() {}
    virtual void get(T& a, T& b, T& c) = 0;
};

template <class T>
class TupleSacriFactory
{
public:
    vector<array<T, 2>> tuples;

    virtual ~TupleSacriFactory() {}
    virtual void get(T& a, T& b) = 0;
};

template <class T>
class SingleSacriFactory
{
public:
    virtual ~SingleSacriFactory() {}
    virtual void get(T& a) = 0;
};

template <class T>
void triple_checking(const Player& P, MacCheck<T>& MC, int nm,
        int output_thread, TripleSacriFactory<Share<T> >& factory,
        bool write_output = true, bool clear = true, string dir = PREP_DIR);
template <class T>
void inverse_checking(const Player& P, MacCheck<T>& MC, int nr,
        int output_thread, TripleSacriFactory<Share<T> >& triple_factory,
        TupleSacriFactory<Share<T> >& inverse_factor,
        bool write_output = true, bool clear = true, string dir = PREP_DIR);
template <class T>
void square_checking(const Player& P, MacCheck<T>& MC, int ns,
        int output_thread, TupleSacriFactory<Share<T> >& factory,
        bool write_output = true, bool clear = true, string dir = PREP_DIR);
void bit_checking(const Player& P, MacCheck<Gfp>& MC, int nb,
        int output_thread, TupleSacriFactory<Share<Gfp> >& square_factory,
        SingleSacriFactory<Share<Gfp> >& bit_factory, bool write_output = true,
        bool clear = true, string dir = PREP_DIR);

template <class T>
inline string file_completion(const T& dummy = {})
{
  (void)dummy;
  return { T::type_char() };
}
