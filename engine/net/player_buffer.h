/*
 * net/player_buffer.h
 *
 */

#pragma once

#include "support/int.h"

class PlayerBuffer
{
public:
    Octet* data;
    size_t size;

    PlayerBuffer() :
            data(0), size(0)
    {
    }

    PlayerBuffer(Octet* data, size_t size) :
            data(data), size(size)
    {
    }
};
