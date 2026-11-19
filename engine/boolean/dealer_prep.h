/*
 * boolean/dealer_prep.h
 *
 */

#pragma once

#include "protocols/dealer/dealer_prep.h"
#include "protocols/common/protocol_set.h"
#include "boolean/shiftable_triple_buffer.h"
#include "boolean/semi_secret.h"

namespace GC
{
class DealerPrep : public BufferPrep<DealerSecret>, ShiftableTripleBuffer<DealerSecret>
{
    Player* P;

public:
    DealerPrep(DataPositions& usage, int = -1) :
            BufferPrep<DealerSecret>(usage), P(0)
    {
    }

    void set_protocol(DealerSecret::Protocol& protocol)
    {
        P = &protocol.P;
        BufferPrep<DealerSecret>::P = P;
    }

    void buffer_triples()
    {
        ProtocolSetup<DealerShare<BitVec>> setup(*P);
        ProtocolSet<DealerShare<BitVec>> set(*P, setup);
        int buffer_size = DIV_CEIL(
                BaseMachine::batch_size<DealerSecret>(DATA_TRIPLE),
                DealerSecret::default_length);
        set.preprocessing.buffer_extra(DATA_TRIPLE, buffer_size);
        for (int i = 0; i < buffer_size; i++)
        {
            auto triple = set.preprocessing.get_triple(
                    DealerSecret::default_length);
            this->triples.push_back({{triple[0], triple[1], triple[2]}});
        }
    }

    void buffer_bits()
    {
        SeededPrng G;
        if (not DealerInput<DealerSecret>(*P).is_dealer())
            for (int i = 0; i < OnlineOptions::singleton.batch_size; i++)
                this->bits.push_back(G.get_bit());
        else
            this->bits.resize(
                    this->bits.size() + OnlineOptions::singleton.batch_size);
    }

    void get(Dtype type, DealerSecret* data)
    {
        BufferPrep<DealerSecret>::get(type, data);
    }

    array<DealerSecret, 3> get_triple_no_count(int n_bits)
    {
        if (n_bits == -1)
            n_bits = DealerSecret::default_length;
        return ShiftableTripleBuffer<DealerSecret>::get_triple_no_count(n_bits);
    }
};

}
