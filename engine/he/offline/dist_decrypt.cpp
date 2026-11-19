
#include "he/offline/dist_decrypt.h"
#include "he/core/p2_data.h"
#include "protocols/common/mac_check.h"

template<class FD>
DistDecrypt<FD>::DistDecrypt(const Player& P, const FHE_SK& share,
    const FHE_PK& pk, const FD& FTD) :
    P(P), share(share), pk(pk), mf(FTD), f(FTD)
{
  vv.resize(pk.get_parameters().phi_m());
  vv1.resize(pk.get_parameters().phi_m());
  // extra limb for operations
  Bigint limit = pk.get_parameters().p0() << 64;
  vv.allocate_slots(limit);
  vv1.allocate_slots(limit);
  mf.allocate_slots(pk.p() << 64);
}

class ModuloTreeSum : public TreeSum<Bigint>
{
  typedef TreeSum<Bigint> Super;

  Bigint modulo;

  void post_add_process(vector<Bigint>& values)
  {
    for (auto& v : values)
      v %= modulo;
  }

public:
  ModuloTreeSum(Bigint modulo) :
      modulo(modulo)
  {
  }

  void run(vector<Bigint>& values, const Player& P)
  {
    lengths.resize(values.size(), numBytes(modulo));
    Super::run(values, P);
  }
};

template<class FD>
Plaintext_<FD>& DistDecrypt<FD>::run(const Ciphertext& ctx, bool new_ciphertext)
{
  CODE_LOCATION
  const FheParameters& params=ctx.get_parameters();

  share.dist_decrypt_1(vv, ctx,P.my_num(),P.num_players());

  if (not new_ciphertext)
    intermediate_step();

  if ((int)vv.size() != params.phi_m())
    throw length_error("wrong length of ring element");

  size_t length = numBytes(pk.get_parameters().p0());

  if (OnlineOptions::singleton.direct)
    {
      // Now pack into an OctetStream for broadcasting
      vector<OctetStream> os(P.num_players());

      for (int i=0; i<params.phi_m(); i++)
        { (os[P.my_num()]).store(vv[i], length); }

      // Broadcast and Receive the values
      P.broadcast_receive(os);

      // Reconstruct the value mod p0 from all shares
      vv1.resize(params.phi_m());
      for (int i = 0; i < P.num_players(); i++)
        {
          if (i != P.my_num())
            {
              for (int j = 0; j < params.phi_m(); j++)
                {
                  os[i].get(vv1[j], length);
                }
              share.dist_decrypt_2(vv, vv1);
            }
        }
    }
  else
    {
      ModuloTreeSum(params.p0()).run(vv, P);
    }

  // Now get the final message
  Bigint mod=params.p0();
  mf.set_polynomial_mod(vv,mod);
  return mf;
}


template class DistDecrypt<FftData>;
template class DistDecrypt<P2Data>;


