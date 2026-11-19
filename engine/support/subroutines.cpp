
#include "support/subroutines.h"

#include "support/random.h"
#include "support/exceptions.h"
#include "support/commit.h"
#include "support/code_locations.h"
#include "numeric/z2k.h"
#include "numeric/gfp.h"
#include "numeric/gf2n.h"

/* To ease readability as I re-write this program the following conventions
 * will be used.
 *   For a variable v index by a player i
 *   Comm_v[i] is the commitment string for player i
 *   Open_v[i] is the opening data for player i
 */


// Special version for OctetStreams
void Commit(vector< vector<OctetStream> >& communication_data,
            vector<OctetStream>& open_data,
            const vector< vector<OctetStream> >& data,const Player& P,int num_runs)
{
  CODE_LOCATION
  int my_number=P.my_num();
  for (int i=0; i<num_runs; i++)
     { communication_data[i].resize(P.num_players());
       Commit(communication_data[i][my_number],open_data[i],data[i][my_number],my_number);
       P.broadcast_receive(communication_data[i]);
     }
}




// Special version for OctetStreams
void Open(vector< vector<OctetStream> >& data,
          const vector< vector<OctetStream> >& communication_data,
          const vector<OctetStream>& my_open_data,
          const Player& P,int num_runs,int dont)
{
  CODE_LOCATION
  int my_number=P.my_num();
  int num_players=P.num_players();
  vector<OctetStream> open_data(num_players);
  for (int i=0; i<num_runs; i++)
     { if (i!=dont)
        { open_data[my_number]=my_open_data[i];
          P.broadcast_receive(open_data);
          for (int j=0; j<num_players; j++) 
            { if (j!=my_number)
	        { if (!Open(data[i][j],communication_data[i][j],open_data[j],j))
	             { throw InvalidCommitment(); }
	        }
	    }
        }
     }
}


void Open(vector< vector<OctetStream> >& data,
          const vector< vector<OctetStream> >& communication_data,
          const vector<OctetStream>& my_open_data,
          const vector<int> open,
          const Player& P,int num_runs)
{
  CODE_LOCATION
  int my_number=P.my_num();
  int num_players=P.num_players();
  vector<OctetStream> open_data(num_players);
  for (int i=0; i<num_runs; i++)
     { if (open[i]==1)
        { open_data[my_number]=my_open_data[i];
          P.broadcast_receive(open_data);
          for (int j=0; j<num_players; j++) 
            { if (j!=my_number)
	         { if (!Open(data[i][j],communication_data[i][j],open_data[j],j))
	             { throw InvalidCommitment(); }
	        }
	    }
        }
     }
}





void commit_to_challenge(vector<unsigned int>& e,
                         vector<OctetStream>& communication_e,vector<OctetStream>& open_e,
                         const Player& P,int num_runs)
{
  CODE_LOCATION
  PRNG G;
  G.re_seed();

  e.resize(P.num_players());
  communication_e.resize(P.num_players());
  open_e.resize(P.num_players());
  
  e[P.my_num()]=G.get_uint(num_runs);
  OctetStream ee; ee.store(e[P.my_num()]);
  Commit(communication_e[P.my_num()],open_e[P.my_num()],ee,P.my_num());
  P.broadcast_receive(communication_e);
}


int open_challenge(vector<unsigned int>& e,vector<OctetStream>& open_e,
                   const vector<OctetStream>& communication_e,
                   const Player& P,int num_runs)
{
  CODE_LOCATION
  // Now open the challenge commitments and determine which run was for real
  P.broadcast_receive(open_e);
  
  int challenge=0;
  OctetStream ee;
  for (int i = 0; i < P.num_players(); i++)
    { if (i != P.my_num())
        { if (!Open(ee,communication_e[i],open_e[i],i))
	     { throw InvalidCommitment(); }
          ee.get(e[i]);
          }
      challenge+=e[i];
    }
  challenge = challenge % num_runs;

  return challenge;
}


void create_random_seed(Octet* seed, const PlayerBase& P, int len,
    const vector<bool>& parties)
{
  CODE_LOCATION

  vector<bool> my_parties = parties;
  if (my_parties.empty())
    my_parties.resize(P.num_players(), true);

  PRNG G;
  G.re_seed();
  vector<OctetStream> e(P.num_players());
  vector<OctetStream> communication_e(P.num_players());
  vector<OctetStream> open_e(P.num_players());

  G.get_octet_stream(e[P.my_num()],len);
  Commit(communication_e[P.my_num()],open_e[P.my_num()],e[P.my_num()],P.my_num());
  P.partial_broadcast(my_parties, my_parties, communication_e);

  P.partial_broadcast(my_parties, my_parties, open_e);

  memset(seed,0,len*sizeof(Octet));
  for (int i = 0; i < P.num_players(); i++)
    if (my_parties.at(i))
      {
        if (i != P.my_num())
          {
            if (!Open(e[i], communication_e[i], open_e[i], i))
              throw InvalidCommitment();
          }
        for (int j = 0; j < len; j++)
          seed[j] ^= *e[i].consume(1);
      }
    else
      {
        if (open_e[i].get_length() > 0)
          throw runtime_error("unexpected information from " + to_string(i));
      }
}


void commit_and_open_(vector<OctetStream>& datas, const Player& P, Coordinator& coordinator)
{
  CODE_LOCATION
  vector<OctetStream> communication_data(P.num_players());
  vector<OctetStream> open_data(P.num_players());

  Commit(communication_data[P.my_num()],open_data[P.my_num()],datas[P.my_num()],P.my_num());
  P.broadcast_receive(communication_data);

  coordinator.wait(P.get_id());
  P.broadcast_receive(open_data);

  for (int i = 0; i < P.num_players(); i++)
    { if (i != P.my_num())
        { if (!Open(datas[i],communication_data[i],open_data[i],i))
             { throw InvalidCommitment(); }
        }
    }
}


void commit_to_seeds(vector<PRNG>& G,
                     vector< vector<OctetStream> >& seeds,
                     vector< vector<OctetStream> >& communication_seeds,
		     vector<OctetStream>& open_seeds,
		     const Player& P,int num_runs)
{
  CODE_LOCATION
  seeds.resize(num_runs);
  communication_seeds.resize(num_runs);
  open_seeds.resize(num_runs);
  for (int i=0; i<num_runs; i++)
    { G[i].re_seed();
      seeds[i].resize(P.num_players());
      communication_seeds[i].resize(P.num_players());
      open_seeds[i].resize(P.num_players());
      seeds[i][P.my_num()].reset_write_head();
      seeds[i][P.my_num()].append(G[i].get_seed(),SEED_SIZE);
    }
  Commit(communication_seeds,open_seeds,seeds,P,num_runs);
}
