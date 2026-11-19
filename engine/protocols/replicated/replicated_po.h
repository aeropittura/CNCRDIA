/*
 * protocols/replicated/replicated_po.h
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep_po.h"

template<class T>
class ReplicatedPo : public MaliciousRepPo<T>
{
public:
    ReplicatedPo(Player& P) :
            MaliciousRepPo<T>(P)
    {
    }

    void send(int player);
    void receive();
};
