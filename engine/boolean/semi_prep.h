/*
 * boolean/semi_prep.h
 *
 */

#pragma once

#include "protocols/common/replicated_prep.h"
#include "ot/mascot_params.h"
#include "boolean/semi_secret.h"
#include "boolean/shiftable_triple_buffer.h"
#include "boolean/semi_secure_shuffle.h"

template<class T> class Beaver;
template<class T> class SubProcessor;

namespace GC
{

template<class T> class ShareThread;

class SemiPrep : public BufferPrep<SemiSecret>, ShiftableTripleBuffer<SemiSecret>
{
    SemiSecret::TripleGenerator* triple_generator;
    MascotParameters params;

    SeededPrng secure_prng;

    vector<array<SemiSecret, 3>> mixed_triples;

    SemiSecureTripleState* state;

    OtExtensionWithMatrix* rot_ext;

public:
    SemiPrep(DataPositions& usage, bool = true);
    SemiPrep(void*, DataPositions& usage) : SemiPrep(usage) {}
    ~SemiPrep();

    void set_protocol(SemiSecret::Protocol& protocol);

    void buffer_triples();
    void buffer_bits();

    void buffer_squares() { throw NotImplemented(); }
    void buffer_inverses() { throw NotImplemented(); }

    array<SemiSecret, 3> get_mixed_triple(int n);

    void get(Dtype type, SemiSecret* data)
    {
        BufferPrep<SemiSecret>::get(type, data);
    }

    array<SemiSecret, 3> get_triple_no_count(int n_bits)
    {
        if (n_bits == -1)
            n_bits = SemiSecret::default_length;
        return ShiftableTripleBuffer<SemiSecret>::get_triple_no_count(n_bits);
    }

    void buffer_personal_triples(vector<array<SemiSecret, 3>>&, size_t, size_t)
    {
        throw NotImplemented();
    }
};

} /* namespace GC */
