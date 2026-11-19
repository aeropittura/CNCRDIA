/*
 * vm/private_output.h
 *
 */

#pragma once

#include <deque>
using namespace std;

template<class T> class SubProcessor;

template<class T>
class PrivateOutput
{
    typedef typename T::OpenType OpenType;

    SubProcessor<T>& proc;
    typename T::MacCheck MC;
    deque<OpenType> masks;

public:
    PrivateOutput(SubProcessor<T>& proc);
    ~PrivateOutput();

    void prepare_sending(const T& source, int player);
    void exchange();
    typename T::clear finalize(int player);
};
