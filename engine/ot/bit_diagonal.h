/*
 * ot/bit_diagonal.h
 *
 */

#pragma once

#include "numeric/square.h"
#include "numeric/bit_vec.h"

class BitDiagonal : public Square<BitVec>
{
public:
    static int size()
    {
        return 8 * BitVec::size();
    }

    void pack(octetStream& os) const;
    void unpack(octetStream& os);
};
