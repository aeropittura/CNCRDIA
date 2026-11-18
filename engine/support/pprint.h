#pragma once

#include <iostream>
#include <iomanip>

using namespace std;

inline void pprint_bytes(const char* label, unsigned char* bytes, int len,
        ostream& out = cout)
{
    out << label << ": ";
    for (int j = 0; j < len; j++)
        out << setfill('0') << setw(2) << hex << (int) bytes[j];
    out << dec << endl;
}
