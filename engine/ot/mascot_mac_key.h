/*
 * ot/mascot_mac_key.h
 *
 */

#pragma once

template<class ShareType>
string get_ot_secrets_filename(const Player& P);

template<class T>
class MascotMacKey : public T
{
    MascotMacKey(const MascotMacKey&) = default;

public:
    OtTripleSetup ot;

    MascotMacKey() = default;

    MascotMacKey& operator=(MascotMacKey&&) = default;

    template<class ShareType>
    void read_or_generate(Player& P);

    void pack(OctetStream& os) const
    {
        T::pack(os);
        ot.pack(os);
    }

    void unpack(OctetStream& os)
    {
        T::unpack(os);
        ot.unpack(os);
    }

    template<class U = T>
    MascotMacKey<U> get_fresh()
    {
        MascotMacKey<U> res;
        res.U::operator=(U(*this));
        res.ot = ot.get_fresh();
        return res;
    }
};
