/*
 * support/subroutines.hpp
 *
 */

#pragma once

#include "support/subroutines.h"
#include "support/code_locations.h"

template<class T>
void create_random(T& ans,const Player& P)
{
  CODE_LOCATION
  PRNG G;
  G.re_seed();
  vector<T> e(P.num_players());
  vector<OctetStream> communication_e(P.num_players());
  vector<OctetStream> open_e(P.num_players());

  e[P.my_num()].randomize(G);
  OctetStream ee;
  e[P.my_num()].pack(ee);
  Commit(communication_e[P.my_num()],open_e[P.my_num()],ee,P.my_num());
  P.broadcast_receive(communication_e);

  P.broadcast_receive(open_e);

  ans.assign_zero();
  for (int i = 0; i < P.num_players(); i++)
    { if (i != P.my_num())
        { if (!Open(ee,communication_e[i],open_e[i],i))
             { throw InvalidCommitment(); }
          e[i].unpack(ee);
          }
      ans += e[i];
    }
}
