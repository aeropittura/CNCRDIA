/*
 * vm/prep_buffer.h
 *
 */

#pragma once

#include "support/buffer.h"

template<class T, class U = T, class V = T>
class PrepBuffer : public BufferOwner<T, U, V>
{
    int num_players;
    string fake_options;

public:
    PrepBuffer() :
            num_players(0)
    {
    }

    void setup(int num_players, const string& filename, int tuple_length,
            const string& type_string = "", const char* data_type = "")
    {
        this->num_players = num_players;
        fake_options = V::template protocol_fake_options<typename V::OpenType>();
        BufferOwner<T, U, V>::setup(filename, tuple_length, type_string, data_type);
    }

    void input(U& a)
    {
        try
        {
            BufferOwner<T, U, V>::input(a);
        }
        catch (exception& e)
        {
            throw PrepSetupError(e.what(), num_players, fake_options);
        }
    }
};
