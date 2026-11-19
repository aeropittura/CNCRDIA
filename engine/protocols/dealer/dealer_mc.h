/*
 * protocols/dealer/dealer_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"
#include "net/all_but_last_player.h"

template<class T>
class DealerOpener : public MacCheckBase<T>
{
    typedef SemiOpener<SemiShare<typename T::clear>> InternalType;
    InternalType& internal;
    AllButLastPlayer* sub_player;

public:
    DealerOpener(typename T::MacKeyType = {}, int = 0, int = 0);
    DealerOpener(InternalType& internal);
    ~DealerOpener();

    void initialize_open(const Player& P, int n = 0);
    void prepare_open(const T& secret, int n_bits = -1);
    void exchange(const Player& P);
    typename T::OpenType finalize_raw();
    array<typename T::OpenType*, 2> finalize_several(size_t n);

    DealerOpener& get_part_opener()
    {
        return *this;
    }
};

template<class T>
class DirectDealerOpener : public DealerOpener<T>
{
public:
    DirectDealerOpener(typename T::MacKeyType = {});
};
