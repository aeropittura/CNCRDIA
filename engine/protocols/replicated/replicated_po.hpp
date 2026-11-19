/*
 * protocols/replicated/replicated_po.hpp
 *
 */

#pragma once

#include "protocols/replicated/replicated_po.h"

#include "protocols/replicated/malicious_rep_po.hpp"

template<class T>
void ReplicatedPo<T>::send(int player)
{
    if (this->P.get_offset(player) == 2)
        this->P.send_to(player, this->to_send);
}

template<class T>
void ReplicatedPo<T>::receive()
{
    this->P.receive_relative(1, this->to_receive[0]);
}
