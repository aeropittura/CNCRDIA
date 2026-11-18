/*
 * SPDZ2^(64+48).cpp
 *
 */

#include "parties/shared/spdz2k.hpp"

template class Machine<Spdz2kShare<64, 64>, Share<gf2n>>;
template int OnlineMachine::run<Spdz2kShare<64, 64>, Share<gf2n>>();
