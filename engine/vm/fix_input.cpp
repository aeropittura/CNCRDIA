/*
 * vm/fix_input.cpp
 *
 */

#include "vm/fix_input.h"

#include <math.h>

template<>
void FixInput_<Integer>::read(std::istream& in, const ArgVector::value_type* params)
{
    double x;
    in >> x;
    items[0] = round(x * exp2(*params));
}

template<>
void FixInput_<Bigint>::read(std::istream& in, const ArgVector::value_type* params)
{
#ifdef HIGH_PREC_INPUT
    mpf_class x;
    in >> x;
    items[0] = x << *params;
#else
    double x;
    in >> x;
    items[0] = round(x * exp2(*params));
#endif
}
