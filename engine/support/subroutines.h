#pragma once

/* Defines subroutines for use in both key_gen and Offline phase
 * Mainly focused around commiting and decommitting to various
 * bits of data
 */

#include "support/random.h"
#include "net/player.h"
#include "support/commit.h"
#include "support/coordinator.h"

#include "support/subroutines.hpp"

/* Run just the Open Protocol for data[i][j] of type OctetStream
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On output data[i][j] contains all the data
 * If dont!=-1 then dont open this run
 */
void Open(vector< vector<OctetStream> >& data,
          const vector< vector<OctetStream> >& communication_data,
          const vector<OctetStream>& my_open_data,
          const Player& P,int num_runs,int dont=-1);

/* This one takes a vector open which contains 0 and 1
 * If 1 then we open this value, otherwise we do not
 */
void Open(vector< vector<OctetStream> >& data,
          const vector< vector<OctetStream> >& communication_data,
          const vector<OctetStream>& my_open_data,
          const vector<int> open,
          const Player& P,int num_runs);




/* This runs the Commit and Open Protocol for data[i][j] of type T
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On input data[i][j] is only defined for j=my_number
 */
template<class T>
void Commit_And_Open(vector< vector<T> >& data,const Player& P,int num_runs);

template<class T>
void Commit_And_Open(vector<T>& data, const Player& P, Coordinator& coordinator);

void commit_and_open_(vector<OctetStream>& datas, const Player& P, Coordinator& coordinator);

template<class T>
void Commit_And_Open(vector<T>& data, const Player& P, Coordinator& coordinator)
{
  vector<OctetStream> datas(P.num_players());
  data[P.my_num()].pack(datas[P.my_num()]);
  commit_and_open_(datas, P, coordinator);
  for (int i = 0; i < P.num_players(); i++)
    data[i].unpack(datas[i]);
}


template<class T>
void transmit_data(vector< vector<T> >& data,const Player& P,int num_runs);

/* Functions to Commit and Open a Challenge Value */
void commit_to_challenge(vector<unsigned int>& e,
                         vector<OctetStream>& communication_e,vector<OctetStream>& open_e,
                         const Player& P,int num_runs);

int open_challenge(vector<unsigned int>& e,vector<OctetStream>& open_e,
                   const vector<OctetStream>& communication_e,
                   const Player& P,int num_runs);


/* Function to create a shared random value for T=gfp/Gf2n */
template<class T>
void create_random(T& ans,const Player& P);

template<class T>
T create_random(const Player& P)
{
  T res;
  create_random(res, P);
  return res;
}

/* Produce a random seed of length len */
void create_random_seed(Octet* seed, const PlayerBase& P, int len,
    const vector<bool>& parties = {});



/* Functions to Commit to Seed Values 
 *  This also initialises the PRNG's in G
 */
void commit_to_seeds(vector<PRNG>& G,
                     vector< vector<OctetStream> >& seeds,
                     vector< vector<OctetStream> >& communication_seeds,
		     vector<OctetStream>& open_seeds,
		     const Player& P,int num_runs);



/* Run just the Commit Protocol for data[i][j] of type T
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On input data[i][j] is only defined for j=my_number
 */
template<class T>
void Commit(vector< vector<OctetStream> >& communication_data,
            vector<OctetStream>& open_data,
            const vector< vector<T> >& data,const Player& P,int num_runs)
{
  OctetStream os;
  int my_number=P.my_num();
  for (int i=0; i<num_runs; i++)
     { os.reset_write_head();
       data[i][my_number].pack(os);
       communication_data[i].resize(P.num_players());
       Commit(communication_data[i][my_number],open_data[i],os,my_number);
       P.broadcast_receive(communication_data[i]);
     }
}


/* Run just the Open Protocol for data[i][j] of type T
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On output data[i][j] contains all the data
 * If dont!=-1 then dont open this run
 */
template<class T>
void Open(vector< vector<T> >& data,
          const vector< vector<OctetStream> >& communication_data,
          const vector<OctetStream>& my_open_data,
          const Player& P,int num_runs,int dont=-1)
{
  OctetStream os;
  int my_number=P.my_num();
  int num_players=P.num_players();
  vector<OctetStream> open_data(num_players);
  for (int i=0; i<num_runs; i++)
     { if (i!=dont)
        { open_data[my_number]=my_open_data[i];
          P.broadcast_receive(open_data);
          for (int j=0; j<num_players; j++)
            { if (j!=my_number)
                { if (!Open(os,communication_data[i][j],open_data[j],j))
                     { throw InvalidCommitment(); }
                  os.reset_read_head();
                  data[i][j].unpack(os);
                }
            }
        }
     }
}
