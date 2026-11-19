
#include "support/subroutines.h"
#include "he/core/rq_element.h"
#include "he/core/ciphertext.h"
#include "support/commit.h"

/* This runs the Commit and Open Protocol for data[i][j] of type T
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On input data[i][j] is only defined for j=my_number
 */
template<class T>
void Commit_And_Open(vector< vector<T> >& data,const Player& P,int num_runs)
{
  CODE_LOCATION
  int num_players=P.num_players();
  vector< vector<OctetStream> > communication_data(num_runs, vector<OctetStream>(num_players));
  vector<OctetStream> open_data(num_runs);
  Commit(communication_data,open_data,data,P,num_runs);
  Open(data,communication_data,open_data,P,num_runs);
}



/* Just transmit data[i][j] of type T
 *   0 <= i < num_runs
 *   0 <= j < num_players
 * On output data[i][j] contains all the data
 */
template<class T>
void transmit_data(vector< vector<T> >& data,const Player& P,int num_runs)
{
  CODE_LOCATION
  int my_number=P.my_num();
  int num_players=P.num_players();
  for (int i=0; i<num_runs; i++)
    {
      vector<OctetStream> t_data(num_players);
      data[i][my_number].pack(t_data[my_number]);
      P.broadcast_receive(t_data);
      for (int j=0; j<num_players; j++)
        if (j!=my_number)
          data[i][j].unpack(t_data[j]);
    }
}



template void Commit_And_Open(vector< vector<RqElement> >& data,const Player& P,int num_runs);
template void Commit_And_Open(vector< vector<Ciphertext> >& data,const Player& P,int num_runs);

template void transmit_data(vector< vector<RqElement> >& data,const Player& P,int num_runs);
template void transmit_data(vector< vector<Ciphertext> >& data,const Player& P,int num_runs);

