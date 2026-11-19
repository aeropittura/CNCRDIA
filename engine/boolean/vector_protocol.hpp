/*
 * boolean/vector_protocol.hpp
 *
 */

#pragma once

#include "boolean/vector_protocol.h"

namespace GC
{

template<class T>
VectorProtocol<T>::VectorProtocol(Player& P) :
        part_protocol(P), P(P)
{
}

template<class T>
void VectorProtocol<T>::init(Preprocessing<T>& prep,
        typename T::MacCheck& MC)
{
    part_protocol.init(prep.get_part(), MC.get_part_opener());
}

template<class T>
void VectorProtocol<T>::initialize_mult()
{
    part_protocol.initialize_mult();
}

template<class T>
void VectorProtocol<T>::prepare_mul(const T& x,
        const T& y, int n)
{
    if (n == -1)
        n = T::default_length;
    for (int i = 0; i < n; i++)
        part_protocol.prepare_mul(x.get_reg(i), y.get_reg(i), 1);
}

template<class T>
void VectorProtocol<T>::exchange()
{
    part_protocol.exchange();
}

template<class T>
T VectorProtocol<T>::finalize_mul(int n)
{
    T res;
    finalize_mult(res, n);
    return res;
}

template<class T>
void VectorProtocol<T>::finalize_mult(T& res, int n)
{
    if (n == -1)
        n = T::default_length;
    res.resize_registers(n);
    for (int i = 0; i < n; i++)
        res.get_reg(i) = part_protocol.finalize_mul(1);
}

} /* namespace GC */
