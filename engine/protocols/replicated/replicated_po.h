/*
 * protocols/replicated/replicated_po.h
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep_po.h"

template<class T>
class ReplicatedPO : public MaliciousRepPO<T>
{
public:
    ReplicatedPO(Player& P) :
            MaliciousRepPO<T>(P)
    {
    }

    void send(int player);
    void receive();
};
