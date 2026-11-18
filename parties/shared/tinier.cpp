/*
 * Tinier.cpp
 *
 */

#include "boolean/tiny_mc.h"
#include "boolean/tinier_secret.h"
#include "boolean/vector_input.h"

#include "boolean/share_party.hpp"
#include "boolean/secret.hpp"
#include "boolean/tiny_prep.hpp"
#include "boolean/share_secret.hpp"
#include "boolean/tinier_share_prep.hpp"
#include "boolean/ccd_prep.hpp"
#include "boolean/personal_prep.hpp"
#include "protocols/common/share.hpp"

//template class GC::ShareParty<GC::TinierSecret<gf2n_mac_key>>;
template class GC::CcdPrep<GC::TinierSecret<gf2n_mac_key>>;
template class Preprocessing<GC::TinierSecret<gf2n_mac_key>>;
template class GC::TinierSharePrep<GC::TinierShare<gf2n_mac_key>>;
template class GC::ShareSecret<GC::TinierSecret<gf2n_mac_key>>;
template class TripleShuffleSacrifice<GC::TinierSecret<gf2n_mac_key>>;
