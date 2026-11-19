/*
 * protocols/replicated/malicious_rep_mc.hpp
 *
 */

#pragma once

#include "protocols/replicated/malicious_rep_mc.h"
#include "boolean/machine.h"
#include "numeric/bit_vec.h"

#include "protocols/replicated/replicated_mc.hpp"
#include "protocols/common/mac_check_base.hpp"

#include <stdlib.h>

template<class T>
void MaliciousRepOpener<T>::POpen_Begin(vector<typename T::OpenType>& values,
        const vector<T>& S, const Player& P)
{
    Super::POpen_Begin(values, S, P);
}

template<class T>
HashMaliciousRepOpener<T>::HashMaliciousRepOpener()
{
    reset();
}

template<class T>
void HashMaliciousRepOpener<T>::reset()
{
    hash.reset();
    needs_checking = false;
}

template<class T>
HashMaliciousRepOpener<T>::~HashMaliciousRepOpener()
{
    if (needs_checking)
    {
        cerr << endl << "SECURITY BUG: insufficient checking" << endl;
        terminate();
    }
}

template<class T>
void HashMaliciousRepOpener<T>::POpen(vector<typename T::OpenType>& values,
        const vector<T>& S, const Player& P)
{
    prepare(S, P);
    P.send_receive_all(to_send, to_receive);
    finalize(values, S, P);
}

template<class T>
void HashMaliciousRepOpener<T>::prepare(const vector<T>& secrets, const Player& P)
{
    to_send.resize(3);
    auto& value_buffer = to_send.at(P.get_player(-1));
    auto buffer_size = secrets.size() * T::OpenType::size();
    value_buffer.reset_reserve(buffer_size);
    hash_buffer.reset_reserve(buffer_size);
    for (auto& x : secrets)
    {
        x[0].pack(value_buffer);
        x[1].pack(hash_buffer);
    }
    hash_buffer.hash(to_send[P.get_player(1)]);
}

template<class T>
void HashMaliciousRepOpener<T>::finalize(vector<typename T::OpenType>& values,
        const vector<T>& secrets, const Player& P)
{
    auto& received_values = to_receive.at(P.get_player(1));
    values.clear();
    for (auto& x : secrets)
    {
        values.push_back(
                x.sum() + received_values.template
                get<typename T::OpenType>());
    }
    received_values.hash(hash_buffer);
    if (hash_buffer != to_receive.at(P.get_player(-1)))
        throw MacFail("check hash mismatch");
}

template<class T>
void HashMaliciousRepOpener<T>::POpen_Begin(vector<typename T::OpenType>&,
        const vector<T>& S, const Player& P)
{
    prepare(S, P);
    P.send_all(to_send);
}

template<class T>
void HashMaliciousRepOpener<T>::POpen_End(vector<typename T::OpenType>& values,
        const vector<T>& S, const Player& P)
{
    P.receive_all(to_receive);
    finalize(values, S, P);
}

template<class T>
void HashMaliciousRepOpener<T>::exchange(const Player& P)
{
    POpen(this->values, this->secrets, P);
}

template<class T>
typename T::OpenType HashMaliciousRepOpener<T>::finalize_raw()
{
    return MacCheckBase<T>::finalize_raw();
}

template<class T>
void HashMaliciousRepOpener<T>::update()
{
    hash.update(os);
    needs_checking = true;
}

template<class T>
void HashMaliciousRepOpener<T>::check_for(const typename T::OpenType& value,
        const vector<T>& check_shares, const Player& P)
{
    os.reset_reserve(check_shares.size() * T::size());
    array<int, 3> js;
    for (int i = 0; i < 3; i++)
        js[i] = (i + P.my_num()) % 3;
    for (auto& share : check_shares)
    {
        typename T::value_type shares[3];
        for (int i = 0; i < 3; i++)
        {
            int j = js[i];
            if (j < 2)
                shares[i] = share[j];
            else
                shares[i] = value - share.sum();
        }
        for (auto& x : shares)
            x.pack(os);
    }
    update();
    Check(P);
}

template<class T>
void HashMaliciousRepOpener<T>::Check(const Player& P)
{
    if (needs_checking)
    {
        CODE_LOCATION
        vector<OctetStream> os(P.num_players());
        hash.final(os[P.my_num()]);
        reset();
        P.broadcast_receive(os);
        for (int i = 0; i < P.num_players(); i++)
            if (os[i] != os[P.my_num()])
                throw MacFail("check hash mismatch");
    }
}

template<class T>
void CommunicationMaliciousRepOpener<T>::POpen(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    POpen_Begin(values, S, P);
    POpen_End(values, S, P);
}

template<class T>
void CommunicationMaliciousRepOpener<T>::POpen_Begin(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    assert(T::vector_length == 2);
    (void)values;
    os.resize(2);
    for (auto& o : os)
        o.reset_write_head();
    for (auto& x : S)
        for (int i = 0; i < 2; i++)
            x[i].pack(os[1 - i]);
    P.pass_around(os[0], 1);
    P.pass_around(os[1], 2);
}

template<class T>
void CommunicationMaliciousRepOpener<T>::POpen_End(vector<typename T::clear>& values,
        const vector<T>& S, const Player& P)
{
    (void) P;
    if (os[0] != os[1])
        throw MacFail();
    values.clear();
    for (auto& x : S)
        values.push_back(os[0].template get<BitVec>() + x.sum());
}

template<class T>
void CommunicationMaliciousRepOpener<T>::Check(const Player& P)
{
    (void)P;
}
