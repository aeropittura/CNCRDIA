
#include "support/subroutines.h"
#include "support/exceptions.h"
#include "support/random.h"

#include "he/offline/enc_commit.h"
#include "he/core/p2_data.h"

#include <math.h>

#include <fstream>
using namespace std;

#include "numeric/modp.hpp"

// XXXX File_prefix is only used for active code
#ifndef file_prefix
#define file_prefix "/tmp/"
#endif


template<class T, class FD, class S>
EncCommit<T, FD, S>::~EncCommit()
{
  for (auto& x : timers)
    cout << x.first << ": " << x.second.elapsed() << endl;
}


// Set up for covert
template<class T,class FD,class S>
void EncCommit<T,FD,S>::init(const Player& PP,const FHE_PK& fhepk,int params,condition cc)
{ 
  P=&PP; pk=&fhepk; 
  num_runs=params; cond=cc;
  covert=true;
}


// Set up for active
template<class T,class FD,class S>
void EncCommit<T,FD,S>::init(const Player& PP,const FHE_PK& fhepk,condition cc,const FD& field_d,int thr)
{ 
  covert=false;
  thread=thr;

  const FheParameters& params=fhepk.get_parameters();

  t = DEFAULT_T;
  b = DEFAULT_B;
  TT = b * t;
  // Probability of cheating in one run is...
  //    ee:=(1-b)*log(t)/log(2.0)-b*log(1+log(log(2)))/log(2.0)+log(M)/log(2);
  // Valid prover could fail, so we allow M failures giving
  // prob of valid prover failing of  (1/delta)^M
  M=5;

  P=&PP; pk=&fhepk; cond=cc;     
 

  // Currently cannot deal with cond!=Full for active adversaries
  if (cond!=Full)
    { throw NotImplemented(); }

  c.resize(t);
  m.resize(t, Plaintext<T,FD,S>(field_d));
  for (int i=0; i<t; i++)
    { c[i].resize((*P).num_players(),Ciphertext(params)); 
      m[i].assign_zero(Polynomial);
    }
  cnt=-1;
}



template<class T,class FD,class S>
void EncCommit<T,FD,S>::next_covert(Plaintext<T,FD,S>& mess, vector<Ciphertext>& C) const
{
  covert_generation(mess, C, {size_t(P->num_players()), pk}, P, num_runs, cond);
}

template <class FD>
void covert_generation(Plaintext_<FD>& mess, vector<Ciphertext>& C,
    const vector<const FHE_PK*>& pks, const Player* P, int num_runs, condition cond)
{
  CODE_LOCATION
  const FheParameters& params=(*pks[0]).get_parameters();

  /* Commit to the seeds */
  vector< vector<OctetStream> > seeds(num_runs, vector<OctetStream>((*P).num_players()));
  vector< vector<OctetStream> > communication_seeds(num_runs,  vector<OctetStream>((*P).num_players()));
  vector<OctetStream> open_seeds(num_runs);

  vector<PRNG> G(num_runs);
  commit_to_seeds(G,seeds,communication_seeds,open_seeds,*P,num_runs);

  // Generate the messages and ciphertexts
  vector< Plaintext_<FD> > m(num_runs,mess.get_field());
  vector<Ciphertext>   c(num_runs,params);
  RandomCoins rc(params);
  for (int i=0; i<num_runs; i++)
    { m[i].randomize(G[i],cond);
      rc.generate(G[i]);
      (*pks[P->my_num()]).encrypt(c[i],m[i],rc);
      //cout << "xxxxxxxxxxxxxxxxxxxxx" << endl;
      //cout << i << "\t" << (*P).socket(P.my_num()) << endl;
      //cout << i << "\t" << m[i] << endl;
      //cout << i << "\t" << rc << endl;
      //cout << i << "\t" << c[i] << endl;
    }

  // Broadcast and receive ciphertexts
  vector< vector<OctetStream> > ctx(num_runs, vector<OctetStream> ((*P).num_players()));
  for (int i=0; i<num_runs; i++)
    { c[i].pack(ctx[i][(*P).my_num()]);
      (*P).broadcast_receive(ctx[i]);
    }

  // Compute and commit to the challenge value
  vector<unsigned int> e((*P).num_players());
  vector<OctetStream> communication_e((*P).num_players());
  vector<OctetStream> open_e((*P).num_players());
  commit_to_challenge(e,communication_e,open_e,*P,num_runs);
  
  // Open challenge
  int challenge=open_challenge(e,open_e,communication_e,*P,num_runs);

  // Open seeds, but not the challenge run
  Open(seeds,communication_seeds,open_seeds,*P,num_runs,challenge);

  // Now check all the prior executions
  Plaintext_<FD> mm(mess.get_field());
  Ciphertext cc(params);
  OctetStream occ;
  for (int i=0; i<num_runs; i++)
    { if (i!=challenge)
        { for (int j=0; j<(*P).num_players(); j++)
            { if (j!=(*P).my_num())
	        { PRNG G;
                  G.SetSeed(seeds[i][j].get_data());
	          //cout << "GOT SEED : " << i << " " << j << " " << P.socket(j) << seeds[i][j] << endl;
                  mm.randomize(G,cond);
                  rc.generate(G);
                  (*pks[j]).encrypt(cc,mm,rc);
                  occ.reset_write_head();
                  cc.pack(occ);
                  if (!occ.equals(ctx[i][j]))
                    { //cout << "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
                      //cout << i << " " << j << " " << P.socket(j) << " " << mm << endl;
                      //cout << i << " " << j << " " << P.socket(j) << " " << rc << endl;
                      //cout << i << " " << j << " " << P.socket(j) << " " << cc << endl;
                      throw BadEnccommit(); 
                    }
		}
	    }
        }
    }

  // Set output
  mess=m[challenge];
  C.resize((*P).num_players(),Ciphertext(params));
  for (int j=0; j<(*P).num_players(); j++)
    { if (j!=(*P).my_num())
	{ C[j].unpack(ctx[challenge][j]); }
      else
        { C[j]=c[challenge]; }
    }
}




template<class T,class FD,class S>
void EncCommit<T,FD,S>::next_active(Plaintext<T,FD,S>& mess, vector<Ciphertext>& C) const
{
  const FheParameters& params=(*pk).get_parameters();

  if (not has_left()) { Create_More(); }
  mess=m[cnt];
  C.resize((*P).num_players(),Ciphertext(params));
  for (int i=0; i<(*P).num_players(); i++)
    { C[i]=c[cnt][i]; }
  cnt--;
}

bool check_norm(const RqElement& value, const Bigint& bound)
{
  bool res = (value.infinity_norm() > bound);
  if (res)
    cout << "out of bound by (log) "
        << log2(value.infinity_norm().get_d() / bound.get_d()) << endl;
  return res;
}

template<class T,class FD,class S>
void EncCommit<T,FD,S>::Create_More() const
{
  CODE_LOCATION
  const FheParameters& params=(*pk).get_parameters();

  // Tweak by using p rather than p/2, as we do not center the ciphertexts
  int rhoi=(int) (6*params.get_R());
  Bigint comm = active_mask(params.phi_m(), b, t);
  Bigint bound1=m[0].get_field().get_prime()*comm;
  Bigint bound2=comm;
  Bigint bound3=rhoi*comm;
  Bigint UBound1=2*(bound1-m[0].get_field().get_prime());
  Bigint UBound2=2*(bound2-1);
  Bigint UBound3=2*(bound3-rhoi);

  int my_num=(*P).my_num();
  /* First generate the good ciphertexts and messages for myself */
  PRNG G;
  G.re_seed();
  vector<RandomCoins> rc(t,params);
  for (int i=0; i<t; i++)
    { m[i].randomize(G,cond);
      rc[i].generate(G);
      (*pk).encrypt(c[i][my_num],m[i],rc[i]);
    }

  // Broadcast and receive ciphertexts "c"
  vector< vector<OctetStream> > ctx_c(t, vector<OctetStream> ((*P).num_players()));
  for (int i=0; i<t; i++)
    { c[i][my_num].pack(ctx_c[i][my_num]);
      (*P).broadcast_receive(ctx_c[i]);
      for (int j=0; j<(*P).num_players(); j++)
	{ if (j!=my_num)
	    { c[i][j].unpack(ctx_c[i][j]); }
        }
    }

  vector< vector<OctetStream> > seeds(2*TT, vector<OctetStream>((*P).num_players()));
  vector< vector<OctetStream> > communication_seeds(2*TT,  vector<OctetStream>((*P).num_players()));
  vector< OctetStream > context_delta((*P).num_players());
  Ciphertext ctx_C(params);
  vector<OctetStream> open_seeds(2*TT);
  vector<PRNG> gseed(2*TT);
  vector<int> open(2*TT);
  vector<RqElement> z(4,RqElement(params.FFTD(),polynomial,polynomial));
  vector<OctetStream> communication_data((*P).num_players());

  char filename[1024];
  bool fail=true;
  int iter=0;
  while (fail && iter<M)
    { fail=false;
      // Now commit to seeds for the Delta's
      cout << "\t Create_More : iteration number = " << iter << endl;
      for (int i=0; i<2*TT; i++)
        { for (int j=0; j<(*P).num_players(); j++)
	    { seeds[i][j].reset_write_head(); 
	      communication_seeds[i][j].reset_write_head(); 
            } 
	  open_seeds[i].reset_write_head(); 
        }
      for (int j=0; j<(*P).num_players(); j++)
	{ communication_data[j].reset_write_head();  }
      
      commit_to_seeds(gseed,seeds,communication_seeds,open_seeds,*P,2*TT);

      // Now generate and transmit/receive the Delta's
      RqElement m_delta(RqElement(params.FFTD(),polynomial,polynomial));
      RandomCoins rc_delta(params);
      for (int i=0; i<2*TT; i++)
        { if (cond!=Full) 
             { throw NotImplemented(); }
          else          
             { m_delta.from(UniformGenerator<Bigint>(gseed[i],num_bits(bound1))); }
          rc_delta.generate_uniform(gseed[i],bound2,bound3,bound3);
          Ciphertext Delta(params);
          (*pk).quasi_encrypt(Delta,m_delta,rc_delta);
          for (int j=0; j<(*P).num_players(); j++)
	    { context_delta[j].reset_write_head(); }
          Delta.pack(context_delta[my_num]);
          (*P).broadcast_receive(context_delta);
           
          // Output the context_delta to a file
          snprintf(filename,1024,"%sctx_Delta-%d-%d-%d",file_prefix,my_num,i,thread);
          ofstream outf(filename);
          for (int j=0; j<(*P).num_players(); j++)
	    {
              ctx_C.unpack(context_delta[j]);
              timers["ciphertext writing"].start();
              ctx_C.output(outf);
              timers["ciphertext writing"].stop();
            }
          outf.close();
        }

      // Generate a random seed and use it to determine which Delta's to open
      Octet seed[SEED_SIZE];
      create_random_seed(seed,*P,SEED_SIZE);
      PRNG RG; RG.SetSeed(seed);
    
      RqElement mm(params.FFTD(),polynomial,polynomial);
      Ciphertext cc(params);
      RandomCoins rr(params);
      for (int i=0; i<2*TT; i++) { open[i]=0; }
      for (int i=0; i<TT; i++)
        { int j=RG.get_uint()%(2*TT);
          while (open[j]==1) { j=RG.get_uint()%(2*TT); }
          open[j]=1;
        }
    
      // Now open the seeds we have decided to open
      Open(seeds,communication_seeds,open_seeds,open,*P,2*TT);
    
      // Now check all opened Delta's are OK for all players
      OctetStream occ,context_d;
      for (int i=0; i<2*TT; i++)
        { if (open[i]==1)
           { snprintf(filename,1024,"%sctx_Delta-%d-%d-%d",file_prefix,my_num,i,thread);
             ifstream inpf(filename);
             for (int j=0; j<(*P).num_players(); j++)
               {
                 timers["ciphertext reading"].start();
                 ctx_C.input(inpf);
                 timers["ciphertext reading"].stop();
                 if (j!=my_num)
                   { PRNG G;
                     G.SetSeed(seeds[i][j].get_data());
                     if (cond!=Full) 
                        { throw NotImplemented(); }
                     else   
	                { mm.from(UniformGenerator<Bigint>(G, num_bits(bound1))); }
                     rr.generate_uniform(G,bound2,bound3,bound3);
                     (*pk).quasi_encrypt(cc,mm,rr);
                     occ.reset_write_head();
                     cc.pack(occ);
                     context_d.reset_write_head();
                     ctx_C.pack(context_d);
                     if (!occ.equals(context_d))
                        { cout << "Error opening position " << i << endl; 
                          // Can only get here if Prover is trying to cheat so abort
                          throw BadEnccommit(); 
                        }
                   }
	       }
             inpf.close();
           }
        }

      // Now move all unopened values of context_delta,seeds to the first TT positions
      //   - Done via index
      int count=0;
      vector<int> index(TT);
      for (int i=0; i<2*TT; i++)
        { if (open[i]==0)
	    { index[count]=i;
              count++;
            }
        }

      for (int i=0; i<t && !fail; i++)
        { for (int j=0; j<b && !fail; j++)
	    { // Step a
              PRNG G;
              G.SetSeed(seeds[index[b*i+j]][my_num].get_data());
              if (cond!=Full) 
                { throw NotImplemented();
                }
              else   
	        { m_delta.from(UniformGenerator<Bigint>(G, num_bits(bound1))); }
              rc_delta.generate_uniform(G,bound2,bound3,bound3);
              
              auto vm=m[i].get_iterator();
              z[0].from(vm);
              add(z[0],z[0],m_delta);
              add(rr,rc[i],rc_delta);
              z[1]=rr.u();
              z[2]=rr.v();
              z[3]=rr.w();
	      // Step b
              if (check_norm(z[0], UBound1)) { cout << "F0:" << i << " " << j <<  endl; fail=true; }
              if (check_norm(z[1], UBound2)) { cout << "F1:" << i << " " << j << endl; fail=true; }
              if (check_norm(z[2], UBound3)) { cout << "F2:" << i << " " << j <<  endl; fail=true; }
              if (check_norm(z[3], UBound3)) { cout << "F3:" << i << " " << j <<  endl; fail=true; }
              // Set my data to transmit
              communication_data[my_num].reset_write_head(); 
              if (!fail)
                 { z[0].pack(communication_data[my_num]);
                   z[1].pack(communication_data[my_num]);
                   z[2].pack(communication_data[my_num]);
                   z[3].pack(communication_data[my_num]);
	         }
              (*P).broadcast_receive(communication_data);
	      Ciphertext enc1(params),enc2(params),e_delta(params);
              OctetStream oe1,oe2;

              snprintf(filename,1024,"%sctx_Delta-%d-%d-%d",file_prefix,my_num,index[b*i+j],thread);
              ifstream inpf(filename);
              for (int k=0; k<(*P).num_players(); k++)
                {
                  timers["ciphertext reading"].start();
                  ctx_C.input(inpf);
                  timers["ciphertext reading"].stop();
                  if (k!=my_num)
	            { if (communication_data[k].get_length()==0)
	                { fail=true; }
                      else
	                { z[0].unpack(communication_data[k]);
	                  z[1].unpack(communication_data[k]);
	                  z[2].unpack(communication_data[k]);
	                  z[3].unpack(communication_data[k]);
	                  // Check size (any wrong means proving cheating)
                          if (z[0].infinity_norm()>UBound1) { cout << "R0" << endl; throw BadEnccommit(); }
                          if (z[1].infinity_norm()>UBound2) { cout << "R1" << endl; throw BadEnccommit(); }
                          if (z[2].infinity_norm()>UBound3) { cout << "R2" << endl; throw BadEnccommit(); }
                          if (z[3].infinity_norm()>UBound3) { cout << "R3" << endl; throw BadEnccommit(); }
	                  // Need to encrypt with z0,..,z1 and check equal to ci+delta_{b*i+j}
	                  rr.assign(z[1],z[2],z[3]);
                          (*pk).quasi_encrypt(enc1,z[0],rr);
                          add(enc2,c[i][k],ctx_C);
	                  enc1.pack(oe1);
	                  enc2.pack(oe2);
                          // If this is not true then we have a cheating prover
	                  if (!oe1.equals(oe2))
	                    { cout << "EFail " << k << endl; throw BadEnccommit(); }
                        }
                    }
                }
              inpf.close();
            }
        }
      if (fail) { iter++; }
    }
  if (fail)
    { throw BadEnccommit(); }

  cnt=t-1;
}


template class EncCommit<Gfp,FftData,Bigint>;
template class EncCommit<Gf2nShort,P2Data,int>;

template class EncCommitBase<Gfp,FftData,Bigint>;
template class EncCommitBase<Gf2nShort,P2Data,int>;

