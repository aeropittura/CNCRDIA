/*
 * protocols/replicated/malicious_rep_po.h
 *
 */

#pragma once

#include "net/player.h"

template<class T>
class MaliciousRepPo
{
protected:
    Player& P;
    OctetStream to_send;
    OctetStream to_receive[2];
    PointerVector<T> secrets;

public:
    MaliciousRepPo(Player& P);
    virtual ~MaliciousRepPo() {}

    void prepare_sending(const T& secret, int player);
    virtual void send(int player);
    virtual void receive();
    typename T::clear finalize(const T& secret);
    typename T::clear finalize();
};
