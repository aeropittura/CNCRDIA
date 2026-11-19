/*
 * protocols/common/protocol_setup.h
 *
 */

#pragma once

#include "net/player.h"

/**
 * Global setup for an arithmetic share type
 */
template<class T>
class ProtocolSetup
{
    typename T::MacKeyType mac_key;

public:
    /**
     * @param P communication instance (used for MAC generation if needed)
     * @param prime_length length of prime if computing modulo a prime
     * @param directory location to read MAC if needed
     */
    ProtocolSetup(Player& P, int prime_length = 0, string directory = "")
    {
        // initialize fields
        if (prime_length == 0)
            prime_length = T::clear::MAX_N_BITS;

        T::clear::initialize_default(prime_length);
        T::clear::next::initialize_default(prime_length, false);

        // must initialize MAC key for security of some protocols
        mac_key = T::LivePrep::get_mac_key(P);
        (void) directory;

        T::MacCheck::setup(P);
    }

    /**
     * @param prime modulus for computation
     * @param P communication instance (used for MAC generation if needed)
     * @param directory location to read MAC if needed
     */
    ProtocolSetup(Bigint prime, Player& P, string directory = "")
    {
        static_assert(T::clear::prime_field, "must use computation modulo a prime");

        T::clear::initialize_field(prime);
        T::clear::next::initialize_field(prime, false);

        // must initialize MAC key for security of some protocols
        mac_key = T::LivePrep::get_mac_key(P);
        (void) directory;

        T::MacCheck::setup(P);
    }

    ~ProtocolSetup()
    {
        T::LivePrep::teardown();
        T::MacCheck::teardown();
    }

    typename T::MacKeyType get_mac_key() const
    {
        return mac_key;
    }

    /**
     * Set how much preprocessing is produced at once.
     */
    static void set_batch_size(size_t batch_size)
    {
        OnlineOptions::singleton.batch_size = batch_size;
    }
};

/**
 * Global setup for a binary share type
 */
template<class T>
class BinaryProtocolSetup
{
    typename T::MacKeyType mac_key;

public:
    /**
     * @param P communication instance (used for MAC generation if needed)
     * @param directory location to read MAC if needed
     */
    BinaryProtocolSetup(Player& P, string directory = "")
    {
        T::part_type::OpenType::initialize_field();
        T::MacKeyType::initialize_field();
        mac_key = T::part_type::LivePrep::get_mac_key(P);
        (void) directory;

        T::MacCheck::setup(P);
    }

    ~BinaryProtocolSetup()
    {
        T::MacCheck::teardown();
    }

    typename T::MacKeyType get_mac_key() const
    {
        return mac_key;
    }
};

/**
 * Global setup for an arithmetic share type and the corresponding binary one
 */
template<class T>
class MixedProtocolSetup : public ProtocolSetup<T>
{
public:
    BinaryProtocolSetup<typename T::BitType> binary;

    /**
     * @param P communication instance (used for MAC generation if needed)
     * @param prime_length length of prime if computing modulo a prime
     * @param directory location to read MAC if needed
     */
    MixedProtocolSetup(Player& P, int prime_length = 0, string directory = "") :
            ProtocolSetup<T>(P, prime_length, directory), binary(P, directory)
    {
    }
};
