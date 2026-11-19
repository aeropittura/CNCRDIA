/*
 * ot/mascot_mac_key.hpp
 *
 */

#pragma once

#include "ot/mascot_mac_key.h"
#include "ot/ot_multiplier.hpp"

#include "protocols/common/share.hpp"

template<class ShareType>
string get_ot_secrets_filename(const Player& P)
{
    OctetStream os;
    ShareType::specification(os);
    string filename = string(PREP_DIR) + "/Mascot-Secrets-"
            + ShareType::clear::type_short() + "-"
            + to_string(ShareType::clear::length()) + "-"
            + os.check_sum(20).get_str(16) + "-P" + to_string(P.my_num()) + "-"
            + to_string(P.num_players());
    return filename;
}

template<class T>
template<class ShareType>
void MascotMacKey<T>::read_or_generate(Player& P)
{
    auto filename = get_ot_secrets_filename<ShareType>(P);
    ShareType::set_secrets_filename(filename);
    OctetStream os;

    auto& key = *this;
    string reason;

    try
    {
        os.input(filename);
        unpack(os);
        BitVector key_bits;
        key_bits.set(key);
        if (key.ot.base_receiver_inputs != key_bits)
            throw runtime_error("inconsistent key");
    }
    catch (exception& e)
    {
        reason = e.what();
    }

    try
    {
        Bundle<OctetStream> bundle(P);
        bundle.mine.store(int(reason.empty()));
        bundle.compare(P);
    }
    catch (MismatchAmongParties& e)
    {
        if (reason.empty())
            reason = e.what();
    }

    if (not reason.empty())
    {
        cerr << "No proper key material found in " << filename
                << ", generating from scratch (" << reason << ")" << endl;
        SeededPrng G;
        key.randomize(G);
        key.ot = OtTripleSetup(P, true);
        key.ot.base_receiver_inputs.set(key);
        auto ot_setup = BaseMachine::fresh_ot_setup(P);
        if (not OtExtensionWithMatrix(0).use_kos())
            P.fork("soft");
        for (int i : P.balanced_others())
            OtMultiplierBase::init(key, P, i, ot_setup);
    }

    os.reset_write_head();
    key.get_fresh().pack(os);
    os.output(filename);
    ShareType::set_mac_key(key);
    write_mac_key<typename ShareType::part_type>(P.N, key);
}
