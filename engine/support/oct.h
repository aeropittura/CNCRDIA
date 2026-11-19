#pragma once

typedef unsigned char Octet;

inline void PRINT_OCTET(const Octet* bytes, size_t size) {
    for (size_t i = 0; i < size; ++i)
        cout << hex << (int) bytes[i];
    cout << flush << endl;
}

inline bool OCTETS_EQUAL(const Octet* left, const Octet* right, int size) {
    for (int i = 0; i < size; ++i)
        if (left[i] != right[i])
            return false;
    return true;
}
