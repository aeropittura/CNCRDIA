/*
 * vm/int_input.hpp
 *
 */

#pragma once

#include "vm/int_input.h"

template<class T>
const char* IntInput<T>::NAME = "integer";

template<class T>
void IntInput<T>::read(std::istream& in, const ArgVector::value_type*)
{
    in >> items[0];
}
