#include "parties/shared/spdz.hpp"

#include "protocols/spdz/mascot_prep.hpp"
#include "vm/field_machine.hpp"
#include "numeric/gfp.hpp"

template class FieldMachine<Share, Share, DishonestMajorityMachine>;

template class Machine<Share<Gfp_<0, 2>>, Share<Gf2n>>;
