/*
 * numeric/square.cpp
 *
 */

#include "numeric/square.h"
#include "numeric/bit_vec.h"
#include "numeric/gf2n.h"
#include "numeric/gfp.h"

template<>
void Square<Gf2nShort>::to(Gf2nShort& result, false_type)
{
    Int128 sum;
    for (int i = 0; i < Gf2nShort::degree(); i++)
        sum ^= Int128(rows[i].get()) << i;
    result = sum;
}

template<>
void Square<BitVec>::to(BitVec& result, false_type)
{
    result = 0;
    for (int i = 0; i < N_ROWS; i++)
        result ^= ((rows[i] >> i) & 1) << i;
}
