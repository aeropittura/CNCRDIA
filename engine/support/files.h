/*
 * support/files.h
 *
 */

#pragma once

#include <iostream>

void open_with_check(std::ifstream& stream, const std::string& filename)
{
    stream.open(filename);
    if (not stream.good())
        throw runtime_error("cannot open " + filename);
}
