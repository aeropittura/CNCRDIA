/*
 * protocols/dealer/dealer_mc.hpp
 *
 */

#pragma once

#include "protocols/dealer/dealer_mc.h"

template<class T>
DealerOpener<T>::DealerOpener(typename T::MacKeyType, int, int) :
        DealerOpener(*(new InternalType))
{
}

template<class T>
DirectDealerOpener<T>::DirectDealerOpener(typename T::MacKeyType) :
        DealerOpener<T>(*(new DirectSemiOpener<SemiShare<typename T::clear>>))
{
}

template<class T>
DealerOpener<T>::DealerOpener(InternalType& internal) :
        internal(internal), sub_player(0)
{
}

template<class T>
DealerOpener<T>::~DealerOpener()
{
    delete &internal;
    if (sub_player)
        delete sub_player;
}

template<class T>
void DealerOpener<T>::initialize_open(const Player& P, int n)
{
    if (P.my_num() != P.num_players() - 1)
    {
        if (not sub_player)
            sub_player = new AllButLastPlayer(P);
        internal.initialize_open(P, n);
    }
}

template<class T>
void DealerOpener<T>::prepare_open(const T& secret, int n_bits)
{
    if (sub_player)
        internal.prepare_open(secret, n_bits);
    else
    {
        if (secret != T())
            throw runtime_error("share for dealer should be 0");
    }
}

template<class T>
void DealerOpener<T>::exchange(const Player&)
{
    CODE_LOCATION
    if (sub_player)
        internal.exchange(*sub_player);
}

template<class T>
typename T::OpenType DealerOpener<T>::finalize_raw()
{
    if (sub_player)
        return internal.finalize_raw();
    else
        return {};
}

template<class T>
array<typename T::OpenType*, 2> DealerOpener<T>::finalize_several(size_t n)
{
    assert(sub_player);
    return internal.finalize_several(n);
}
