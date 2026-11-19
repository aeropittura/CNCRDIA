
#include "he/offline/reshare.h"
#include "he/offline/dist_decrypt.h"
#include "he/core/p2_data.h"
#include "support/random.h"

#include "numeric/modp.hpp"

template<class T,class FD,class S>
void Reshare(Plaintext<T,FD,S>& m,Ciphertext& cc,
             const Ciphertext& cm,bool new_ciphertext,
             const Player& P,EncCommitBase<T,FD,S>& EC,
             const FHE_PK& pk,const FHE_SK& share)
{
  DistDecrypt<FD> dd(P, share, pk, m.get_field());
  Reshare(m, cc, cm, new_ciphertext, P, EC, pk, dd);
}

template<class T,class FD,class S>
void Reshare(Plaintext<T,FD,S>& m,Ciphertext& cc,
             const Ciphertext& cm,bool new_ciphertext,
             const Player& P,EncCommitBase<T,FD,S>& EC,
             const FHE_PK& pk,DistDecrypt<FD>& dd)
{
  const FheParameters& params=pk.get_parameters();

  // Step 1
  Ciphertext cf(params);
  Plaintext_<FD>& f = dd.f;
  EC.next(f,cf);

  // Step 2
  // We could be resharing a level 0 ciphertext so adjust if we are
  if (cm.level()==0) { cf.Scale(m.get_field().get_prime()); }
  Ciphertext cmf(params);
  add(cmf,cf,cm);

  // Step 3
  Plaintext_<FD>& mf = dd.mf;
  dd.run(cmf, new_ciphertext);
  
  // Step 4
  if (P.my_num()==0)
    { sub(m,mf,f); }
  else
    { m=f; m.negate(); }
 
  // Step 5
  if (new_ciphertext)
    {
      RandomCoins rc(params);
      pk.encrypt(cc,mf,rc);
      // And again
      if (cf.level()==0) { cc.Scale(m.get_field().get_prime()); }
      sub(cc,cc,cf);
    }
}




template void Reshare(Plaintext<Gfp,FftData,Bigint>& m,Ciphertext& cc,
                      const Ciphertext& cm,bool new_ciphertext,
                      const Player& P,EncCommitBase<Gfp,FftData,Bigint>& EC,
                      const FHE_PK& pk,DistDecrypt<FftData>& dd);

template void Reshare(Plaintext<Gf2nShort,P2Data,int>& m,Ciphertext& cc,
                      const Ciphertext& cm,bool new_ciphertext,
                      const Player& P,EncCommitBase<Gf2nShort,P2Data,int>& EC,
                      const FHE_PK& pk,DistDecrypt<P2Data>& dd);


template void Reshare(Plaintext<Gfp,FftData,Bigint>& m,Ciphertext& cc,
                      const Ciphertext& cm,bool new_ciphertext,
                      const Player& P,EncCommitBase<Gfp,FftData,Bigint>& EC,
                      const FHE_PK& pk,const FHE_SK& share);

template void Reshare(Plaintext<Gf2nShort,P2Data,int>& m,Ciphertext& cc,
                      const Ciphertext& cm,bool new_ciphertext,
                      const Player& P,EncCommitBase<Gf2nShort,P2Data,int>& EC,
                      const FHE_PK& pk,const FHE_SK& share);


