/*
 * protocols/spdzwise/spdzwise_share.hpp
 *
 */

#pragma once

#include "protocols/spdzwise/spdzwise_share.h"

#include "protocols/fake/fake_stuff.hpp"

template<class T>
void SpdzWiseShare<T>::read_or_generate_mac_key(string directory, Player& P, T& mac_key)
{
    CODE_LOCATION
    bool fresh = false;

    try
    {
        read_mac_key(directory, P.N, mac_key);
    }
    catch (MacKeyError&)
    {
        fresh = true;
    }

    try
    {
        // validate MAC key
        typename OpenPartType::MacCheck MC;
        auto masked = typename T::Honest::Protocol(P).get_random() + mac_key;
        MC.open(masked, P);
        MC.Check(P);
    }
    catch (MacFail&)
    {
        fresh = true;
        cerr << "Invalid " << type_string() << " MAC key, generating fresh one" << endl;
    }

    if (fresh)
        mac_key = typename T::Honest::Protocol(P).get_random();

    Super::set_mac_key(mac_key);
}

template<class T>
void SpdzWiseShare<T>::pack(OctetStream& os, bool full) const
{
    Super::pack(os, full);
}

template<class T>
void SpdzWiseShare<T>::pack(OctetStream& os, OpenType factor) const
{
    this->get_share().pack(os, factor);
}
