/*
 * garbled/bmr/register_inline.h
 *
 */

#pragma once

#include "garbled/bmr/common_party.h"
#include "garbled/bmr/party.h"

inline Register::Register() :
        garbled_entry(CommonParty::s().get_n_parties()), external(NO_SIGNAL),
        mask(NO_SIGNAL), keys(CommonParty::s().get_n_parties())
{
}
