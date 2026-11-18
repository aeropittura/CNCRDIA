/*
 * boolean/atlas_secret.cpp
 *
 */

#include "boolean/atlas_secret.h"
#include "boolean/tiny_mc.h"

#include "protocols/shamir/shamir.hpp"
#include "protocols/shamir/shamir_mc.hpp"
#include "protocols/common/mac_check_base.hpp"
#include "boolean/secret.hpp"

namespace GC
{

typename AtlasSecret::MC* AtlasSecret::new_mc(typename AtlasSecret::mac_key_type mac_key)
{
    return new MC(mac_key);
}

AtlasShare::AtlasShare(const AtlasSecret& other) :
        AtlasShare(other.get_bit(0))
{
}

}
