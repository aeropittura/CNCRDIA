/*
 * vm/fix_input.h
 *
 */

#pragma once

#include <iostream>

#include "numeric/bigint.h"
#include "numeric/integer.h"
#include "vm/instruction.h"

template<class T>
class FixInput_
{
public:
    const static int N_DEST = 1;
    const static int N_PARAM = 1;
    const static char* NAME;

    const static int TYPE = 1;

    T items[N_DEST];

    void read(std::istream& in, const ArgVector::value_type* params);
};

template<class T>
const char* FixInput_<T>::NAME = "real number";

#ifdef LOW_PREC_INPUT
typedef FixInput_<Integer> FixInput;
#else
typedef FixInput_<Bigint> FixInput;
#endif
