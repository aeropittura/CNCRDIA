/*
 * boolean/tiny_mc.h
 *
 */

#pragma once

#include "protocols/common/mac_check_base.h"
#include "vm/online_options.h"

namespace GC
{

template<class T>
class TinyOpener : public MacCheckBase<T>
{
    typename T::part_type::MacCheck* part_opener;
    PointerVector<int> sizes;

public:
    static void setup(Player& P)
    {
        T::part_type::MacCheck::setup(P);
    }

    static void teardown()
    {
        T::part_type::MacCheck::teardown();
    }

    TinyOpener(typename T::MacKeyType mac_key)
    {
        this->alphai = mac_key;

        if (OnlineOptions::singleton.direct)
            part_opener = new typename T::part_type::DirectOpener(mac_key);
        else
            part_opener = new typename T::part_type::MacCheck(mac_key);

    }

    ~TinyOpener()
    {
        delete part_opener;
    }

    typename T::part_type::MacCheck& get_part_opener()
    {
        return *part_opener;
    }

    void initialize_open(const Player& P, int n = 0)
    {
        part_opener->initialize_open(P);
        sizes.clear();
        sizes.reserve(n);
    }

    void prepare_open(const T& secret, int = -1)
    {
        for (auto& part : secret.get_registers())
            part_opener->prepare_open(part);
        sizes.push_back(secret.get_registers().size());
    }

    void exchange(const Player& P)
    {
        part_opener->exchange(P);
    }

    typename T::OpenType finalize_raw()
    {
        int n = sizes.next();
        typename T::OpenType opened = 0;
        for (int i = 0; i < n; i++)
            opened += typename T::OpenType(part_opener->finalize_raw().get_bit(0)) << i;
        return opened;
    }

    void Check(const Player& P)
    {
        part_opener->Check(P);
    }
};

} /* namespace GC */
