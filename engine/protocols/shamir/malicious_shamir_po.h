/*
 * protocols/shamir/malicious_shamir_po.h
 *
 */

#pragma once

template<class T>
class MaliciousShamirPo
{
protected:
    Player& P;

    OctetStream to_send;
    vector<OctetStream> to_receive;

    vector<typename T::OpenType> shares;
    typename T::DirectOpener MC;

    int n_relevant_players();

public:
    MaliciousShamirPo(Player& P);

    void prepare_sending(const T& secret, int player);
    void send(int player);
    void receive();
    typename T::clear finalize(const T& secret);
};
