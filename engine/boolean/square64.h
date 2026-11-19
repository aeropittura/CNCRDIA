/*
 * boolean/Square64.h
 *
 */

#pragma once

#include <string.h>
#include <cstdint>
#include "support/int.h"
#include "support/intrinsics.h"

union Square64
{
    __m256i quadrows[16];
    __m128i doublerows[32];
    int64_t rows[64];
    int32_t halfrows[128][2];
    Octet bytes[64][8];

    Square64()
    {
        memset(bytes, 0, sizeof(bytes));
    }

    bool get_bit(int x, int y)
    { return (bytes[x][y/8] >> (y % 8)) & 1; }

    void transpose(int n_rows, int n_cols);

    bool operator!=(const Square64& other);

    void print();
};
