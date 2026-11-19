
#include "he/core/fhe_keys.h"
#include "he/core/ciphertext.h"
#include "he/core/p2_data.h"
#include "he/core/fft_data.h"
#include "support/code_locations.h"
#include "vm/online_options.h"

#include "numeric/modp.hpp"


FHE_SK::FHE_SK(const FHE_PK& pk) : FHE_SK(pk.get_parameters(), pk.p())
{
}

FHE_SK::FHE_SK(const FheParameters& pms) :
    FHE_SK(pms, pms.get_plaintext_modulus())
{
}


FHE_SK& FHE_SK::operator+=(const FHE_SK& c)
{ 
  auto& a = *this;
  auto& b = *this;

  if (a.params!=c.params) { throw ParametersMismatch(); }

  ::add(a.sk,b.sk,c.sk);

  return *this;
}



void key_gen(FHE_PK& PK,FHE_SK& SK,PRNG& G)
{
  if (PK.params!=SK.params) { throw ParametersMismatch(); }
  if (PK.pr!=SK.pr)         { throw PrMismatch(); }

  RqElement sk = PK.sample_secret_key(G);
  SK.assign(sk);
  PK.key_gen(sk, G);
}


FHE_PK::FHE_PK(const FheParameters& pms) :
    FHE_PK(pms, pms.get_plaintext_modulus())
{
}

RqElement FHE_PK::sample_secret_key(PRNG& G)
{
  RqElement sk = FHE_SK(*this).s();
  // Generate the secret key
  sk.from(GaussianGenerator<Bigint>(params->get_dg(), G));
  return sk;
}

void FHE_PK::key_gen(RqElement& sk, PRNG& G, int noise_boost)
{
  RqElement a(*this);
  a.randomize(G);
  partial_key_gen(sk, a, G, noise_boost);
}

void FHE_PK::partial_key_gen(const RqElement& sk, const RqElement& a, PRNG& G,
    int noise_boost)
{
  CODE_LOCATION_NO_SCOPE

  FHE_PK& PK = *this;

  a0 = a;

  // b0=a0*s+p*e0
  RqElement e0((*PK.params).FFTD(),evaluation,evaluation);
  e0.from(GaussianGenerator<Bigint>(params->get_dg(), G, noise_boost));
  mul(PK.b0,PK.a0,sk);
  mul(e0,e0,PK.pr);
  add(PK.b0,PK.b0,e0);

#ifdef CHECK_NOISE
  // strict check not working for GF(2^n)
  PK.check_noise(PK.b0 - PK.a0 * sk, false);
#endif

  if (params->n_mults() > 0)
    {
      // Generating the switching key data
      PK.sw_a.randomize(G);

      // bs=as*s+p*es
      RqElement es((*PK.params).FFTD(),evaluation,evaluation);
      es.from(GaussianGenerator<Bigint>(params->get_dg(), G, noise_boost));
      mul(PK.sw_b,PK.sw_a,sk);
      mul(es,es,PK.pr);
      add(PK.sw_b,PK.sw_b,es);

      // bs=bs-p1*s^2
      // Mult at level 0
      auto s2 = sk * sk;
      s2.mult_by_p1();         // This raises back to level 1
      sub(PK.sw_b,PK.sw_b,s2);
    }
}

void FHE_PK::check_noise(const FHE_SK& SK) const
{
  RqElement sk = SK.s();
  if (params->n_mults() > 0)
    sk.mult_by_p1();
  check_noise(b0 - a0 * sk);
}

void FHE_PK::check_noise(const RqElement& x, bool check_modulo) const
{
  assert(pr != 0);
  vector<Bigint> noise = x.to_vector_bigint();
  Bigint m = 0;
  if (check_modulo)
    cout << "checking multiplicity of noise" << endl;
  for (size_t i = 0; i < noise.size(); i++)
    {
//	  cout << "noise mod pr: " << noise[i] << " pr: " << pr << " " << noise[i] % pr << "\n";
      if (check_modulo and noise[i] % pr != 0)
        {
          cout << i << " " << noise[i] % pr << endl;
          throw runtime_error("invalid public key");
        }
      noise[i] /= pr;
      m = m > noise[i] ? m : noise[i];
    }
#ifdef VERBOSE_KEYGEN
  cerr << "max noise: " << m << endl;
#endif
}


template<class T, class FD, class S>
void FHE_PK::encrypt(Ciphertext& c,
                     const Plaintext<T, FD, S>& mess,const RandomCoins& rc) const
{
  if (&c.get_parameters()!=params)  { throw ParametersMismatch(); }
  if (&rc.get_parameters()!=params) { throw ParametersMismatch(); }
  if (T::characteristic_two ^ (pr == 2))
    throw PrMismatch();

  RqElement mm((*params).FFTD(),polynomial,polynomial);
  mm.from(mess.get_iterator());

  quasi_encrypt(c,mm,rc);
}

void FHE_PK::quasi_encrypt(Ciphertext& c,
                           const RqElement& mess,const RandomCoins& rc) const
{
  CODE_LOCATION_NO_SCOPE

  if (&c.get_parameters()!=params)  { throw ParametersMismatch(); }
  if (&rc.get_parameters()!=params) { throw ParametersMismatch(); }
  assert(pr != 0);

  // c1=a0*u+p*v
  auto c1 = a0 * rc.u() + rc.v() * pr;

  // c0 = b0 * u + p * w + mess
  auto c0 = b0 * rc.u();
  auto edd = rc.w() * pr + mess;
  if (params->n_mults() == 0)
    edd.change_rep(evaluation);
  else
    edd.change_rep(evaluation, evaluation);
  add(c0,c0,edd);

  c.set(c0,c1,*this);
}

template<class FD>
Ciphertext FHE_PK::encrypt(const Plaintext<typename FD::T, FD, typename FD::S>& mess,
    const RandomCoins& rc) const
{
  Ciphertext res(*params);
  encrypt(res, mess, rc);
  return res;
}


template<class FD>
Ciphertext FHE_PK::encrypt(
    const Plaintext<typename FD::T, FD, typename FD::S>& mess) const
{
  return encrypt(RqElement(*params, mess));
}

Ciphertext FHE_PK::encrypt(const RqElement& mess) const
{
  RandomCoins rc(*params);
  PRNG G;
  G.re_seed();
  rc.generate(G);
  Ciphertext res(*params);
  quasi_encrypt(res, mess, rc);
  return res;
}


template<class T, class FD, class S>
void FHE_SK::decrypt(Plaintext<T,FD,S>& mess,const Ciphertext& c) const
{
  if (T::characteristic_two ^ (pr == 2))
    throw PrMismatch();

  RqElement ans = quasi_decrypt(c);
  mess.set_polynomial_mod(ans.get_iterator(), ans.get_modulus());
}

RqElement FHE_SK::quasi_decrypt(const Ciphertext& c) const
{
  CODE_LOCATION_NO_SCOPE

  if (&c.get_parameters()!=params)  { throw ParametersMismatch(); }

  auto ans = c.c0() - c.c1() * sk;
  ans.change_rep(polynomial);
  return ans;
}



Plaintext_<FftData> FHE_SK::decrypt(const Ciphertext& c)
{
  return decrypt(c, params->get_plaintext_field_data<FftData>());
}

template<class FD>
Plaintext<typename FD::T, FD, typename FD::S> FHE_SK::decrypt(const Ciphertext& c, const FD& field_d)
{
  Plaintext<typename FD::T, FD, typename FD::S> res(field_d);
  decrypt_any(res, c);
  return res;
}

template <class FD>
void FHE_SK::decrypt_any(Plaintext_<FD>& res, const Ciphertext& c)
{
  if (sk.level())
      sk.lower_level();
  if (c.level())
    {
      Ciphertext cc = c;
      cc.Scale(res.get_field().get_prime());
      decrypt(res, cc);
    }
  else
    decrypt(res, c);
}





/* Distributed Decryption Stuff */
void FHE_SK::dist_decrypt_1(vector<Bigint>& vv,const Ciphertext& ctx,int player_number,int num_players) const
{
  // Need Ciphertext to be at level 0, so we force this here
  Ciphertext cc=ctx; cc.Scale(pr);

  // First do the basic decryption
  auto dec_sh = cc.c1() * sk;
  if (player_number==0)
    { sub(dec_sh,cc.c0(),dec_sh); }
  else
    { dec_sh.negate(); }

  // Now convert to a vector of bigint's and add the required randomness
  assert(pr != 0);
  Bigint Bd=((*params).B()<<(*params).secp())/(num_players*pr);
  Bd=Bd/2; // make slightly smaller due to rounding issues

  dec_sh.to_vector_bigint(vv);
  if ((int)vv.size() != params->phi_m())
    throw length_error("wrong length of ring element");
  Bigint mod=(*params).p0();
  PRNG G;  G.re_seed();
  Bigint mask;
  Bigint two_bd = 2 * Bd;

  bool verbose = OnlineOptions::singleton.has_option("verbose_dd");
  int max_bits = 0;

  if (verbose)
    cerr << "Random bits in distributed decryption: " << two_bd.num_bits()
        << endl;

  for (int i=0; i<(*params).phi_m(); i++)
    {
      G.random_bnd(mask, two_bd);
      mask -= Bd;
      mask *= pr;
      vv[i] += mask;
      vv[i] %= mod;
      if (vv[i]<0) { vv[i]+=mod; }

      if (verbose)
        max_bits = max(max_bits, vv[i].num_bits());
    }

  if (verbose)
    cerr << "Maximum bits in distributed decryption: " << max_bits << endl;
}


void FHE_SK::dist_decrypt_2(vector<Bigint>& vv,const vector<Bigint>& vv1) const
{
  Bigint mod=(*params).p0();
  for (int i=0; i<(*params).phi_m(); i++)
    {
      vv[i] += vv1[i];
      vv[i] %= mod;
    }
}

void FHE_PK::pack(OctetStream& o) const
{
  o.append((Octet*) "PKPKPKPK", 8);
  a0.pack(o);
  b0.pack(o);
  if (params->n_mults() > 0)
    {
      sw_a.pack(o);
      sw_b.pack(o);
    }
  pr.pack(o);
}

void FHE_PK::unpack(OctetStream& o)
{
  char tag[8];
  o.consume((Octet*) tag, 8);
  if (memcmp(tag, "PKPKPKPK", 8))
    throw runtime_error("invalid serialization of public key");
  a0.unpack(o, *params);
  b0.unpack(o, *params);
  if (params->n_mults() > 0)
    {
      sw_a.unpack(o, *params);
      sw_b.unpack(o, *params);
    }
  pr.unpack(o);
}


bool FHE_PK::operator!=(const FHE_PK& x) const
{
  if ((*params) != *(x.params) or pr != x.pr or a0 != x.a0 or b0 != x.b0
      or sw_a != x.sw_a or sw_b != x.sw_b)
    {
      throw runtime_error("pk");
      return true;
    }
  else
    return false;
}

void FHE_SK::check(const FheParameters& params, const FHE_PK& pk,
        const Bigint& pr) const
{
  if (this->params != &params)
    throw ParametersMismatch();
  if (this->pr != pr)
    throw PrMismatch();
  pk.check(params, pr);
  sk.check(params);
}


template<class FD>
void FHE_SK::check(const FHE_PK& pk, const FD& field_d)
{
  check(*params, pk, field_d.get_prime());
  pk.check_noise(*this);
  if (decrypt(pk.encrypt(Plaintext_<FD>(field_d)), field_d) !=
      Plaintext_<FD>(field_d))
    throw runtime_error("incorrect key pair");
}

void FHE_PK::check(const FheParameters& params, const Bigint& pr) const
{
  if (this->pr != pr)
    throw PrMismatch();
  a0.check(params);
  b0.check(params);

  if (params.n_mults() > 0)
    {
      sw_a.check(params);
      sw_b.check(params);
    }
}

Bigint FHE_SK::get_noise(const Ciphertext& c)
{
  sk.lower_level();
  Ciphertext cc = c;
  if (cc.level())
    cc.Scale();
  RqElement tmp = quasi_decrypt(cc);
  Bigint res;
  Bigint q = tmp.get_modulus();
  Bigint half_q = q / 2;
  for (auto& x : tmp.to_vector_bigint())
    {
//      cout << num_bits(x) << "/" << (x > half_q) << "/" << (x < 0) << " ";
      res = max(res, x > half_q ? x - q : x);
    }
  return res;
}


#define X(FD) \
        template void FHE_PK::encrypt(Ciphertext&, const Plaintext_<FD>& mess, \
                const RandomCoins& rc) const; \
        template Ciphertext FHE_PK::encrypt(const Plaintext_<FD>& mess) const; \
        template Plaintext_<FD> FHE_SK::decrypt(const Ciphertext& c, \
                const FD& field_d); \
        template void FHE_SK::decrypt(Plaintext_<FD>& res, \
		const Ciphertext& c) const; \
        template void FHE_SK::decrypt_any(Plaintext_<FD>& res, \
		const Ciphertext& c); \
        template void FHE_SK::check(const FHE_PK& pk, const FD&);

X(FftData)
X(P2Data)
