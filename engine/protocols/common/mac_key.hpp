/*
 * protocols/common/mac_key.hpp
 *
 */

#pragma once

#include "net/player.h"
#include "numeric/setup.h"

#include "protocols/fake/fake_stuff.hpp"

template<class T>
typename T::MacKeyType read_or_generate_mac_key(const Player& P,
        string directory = "")
{
    if (directory == "")
        directory = get_prep_sub_directory<T>(P.num_players());
    typename T::MacKeyType res;
    T::read_or_generate_mac_key(directory, P, res);
    return res;
}

template<class T>
void check_field_size()
{
    if (T::length() < OnlineOptions::singleton.security_parameter)
        throw FieldTooSmall(T::length(),
                OnlineOptions::singleton.security_parameter);
}
