/*
 * parties/shared/tinier.cpp
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

//template class GC::ShareParty<GC::TinierSecret<Gf2nMacKey>>;
template class GC::CcdPrep<GC::TinierSecret<Gf2nMacKey>>;
template class Preprocessing<GC::TinierSecret<Gf2nMacKey>>;
template class GC::TinierSharePrep<GC::TinierShare<Gf2nMacKey>>;
template class GC::ShareSecret<GC::TinierSecret<Gf2nMacKey>>;
template class TripleShuffleSacrifice<GC::TinierSecret<Gf2nMacKey>>;
