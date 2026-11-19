/*
 * vm/edabit_buffer.h
 *
 */

#pragma once

#include "support/buffer.h"

template<class T>
class EdabitBuffer : public BufferOwner<T, T>
{
    int n_bits;

    int element_length()
    {
        return -1;
    }

public:
    EdabitBuffer(int n_bits = 0) :
            n_bits(n_bits)
    {
    }

    Edabitvec<T> read()
    {
        if (not BufferBase::file)
        {
            if (this->open()->fail())
                throw runtime_error(
                        "error opening " + this->filename
                                + ", have you generated edaBits, "
                                        "for example by running "
                                        "'./Fake-Offline.x -e "
                                + to_string(n_bits)
                                + T::template protocol_fake_options<typename T::clear>()
                                + " ...'?");
        }

        assert(BufferBase::file);
        auto& buffer = *BufferBase::file;
        if (buffer.peek() == EOF)
        {
            this->try_rewind();
        }

        Edabitvec<T> eb;
        eb.input(n_bits, buffer);
        if (buffer.fail())
            throw runtime_error("error reading edaBits");
        return eb;
    }
};
