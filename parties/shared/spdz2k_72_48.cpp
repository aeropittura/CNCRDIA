/*
 * SPDZ2^(64+48).cpp
 *
 */

#include "parties/shared/spdz2k.hpp"

template int OnlineMachine::run<Spdz2kShare<72, 48>, Share<Gf2n>>();
