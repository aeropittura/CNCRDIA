/*
 * parties/shared/spdz2k.cpp
 *
 */

#include "parties/shared/spdz2k.hpp"

#ifdef RING_SIZE
template int OnlineMachine::run<Spdz2kShare<RING_SIZE, SPDZ2K_DEFAULT_SECURITY>, Share<Gf2n>>();
#endif
