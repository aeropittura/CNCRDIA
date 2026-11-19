#pragma once

/* These are standardly generated FHE public and private key pairs */

#include "he/core/rq_element.h"
#include "he/core/fhe_params.h"
#include "he/core/random_coins.h"
#include "he/core/ciphertext.h"
#include "he/core/plaintext.h"

class FHE_PK;
class Ciphertext;

/**
 * BGV secret key.
 * The class allows addition.
 */
class FHE_SK
{
  RqElement sk;
  const FheParameters *params;
  Bigint pr;

  public:

  static false_type optimized_packing;

  static int size() { return 0; }

  const FheParameters& get_parameters() const { return *params; }

  Bigint p() const { return pr; }

  // secret key always on lower level
  void assign(const RqElement& s) { sk=s; sk.lower_level(); }

  FHE_SK(const FheParameters& pms);

  FHE_SK(const FheParameters& pms, const Bigint& p)
    : sk(pms.FFTD(),evaluation,evaluation) { params=&pms; pr=p; }

  FHE_SK(const FHE_PK& pk);

  // Rely on default copy constructor/assignment
  
  const RqElement& s() const { return sk; }

  /// Append to buffer
  void pack(OctetStream& os, int = -1) const { sk.pack(os); pr.pack(os); }

  /// Read from buffer. Assumes parameters are set correctly
  void unpack(OctetStream& os, int = -1)
  { sk.unpack(os, *params); pr.unpack(os); }

  // Assumes Ring and prime of mess have already been set correctly
  // Ciphertext c must be at level 0 or an error occurs
  //            c must have same params as SK
  template<class T, class FD, class S>
  void decrypt(Plaintext<T, FD, S>& mess,const Ciphertext& c) const;

  template <class FD>
  Plaintext<typename FD::T, FD, typename FD::S> decrypt(const Ciphertext& c, const FD& field_d);

  /// Decryption for cleartexts modulo prime
  Plaintext_<FftData> decrypt(const Ciphertext& c);

  template <class FD>
  void decrypt_any(Plaintext_<FD>& mess, const Ciphertext& c);

  RqElement quasi_decrypt(const Ciphertext& c) const;

  // Three stage procedure for Distributed Decryption
  //  - First stage produces my shares
  //  - Second stage adds in another players shares, do this once for each other player
  //  - Third stage outputs the message by executing
  //        mess.set_polynomial_mod(vv,mod)  
  //    where mod p0 and mess is Plaintext<T,FD,S>
  void dist_decrypt_1(vector<Bigint>& vv,const Ciphertext& ctx,int player_number,int num_players) const;
  void dist_decrypt_2(vector<Bigint>& vv,const vector<Bigint>& vv1) const;
  
  friend void key_gen(FHE_PK& PK,FHE_SK& SK,PRNG& G);
  
  /* Add secret keys
   *   Used for adding distributed keys together
   *   a,b,c must have same params otherwise an error
   */

  FHE_SK operator+(const FHE_SK& x) const { FHE_SK res = *this; res += x; return res; }
  FHE_SK& operator+=(const FHE_SK& x);

  bool operator!=(const FHE_SK& x) const { return pr != x.pr or sk != x.sk; }

  void check(const FheParameters& params, const FHE_PK& pk, const Bigint& pr) const;

  template<class FD>
  void check(const FHE_PK& pk, const FD& field_d);

  Bigint get_noise(const Ciphertext& c);

  friend ostream& operator<<(ostream& o, const FHE_SK&) { throw NotImplemented(); return o; }
};


/**
 * BGV public key.
 */
class FHE_PK
{
  RqElement a0,b0;
  RqElement sw_a,sw_b;
  const FheParameters *params;
  Bigint pr;

  public:

  const FheParameters& get_parameters() const { return *params; }

  Bigint p() const { return pr; }

  void assign(const RqElement& a,const RqElement& b,
              const RqElement& sa,const RqElement& sb
             )
	{ a0=a; b0=b; sw_a=sa; sw_b=sb; }

  void assign(const RqElement& a,const RqElement& b)
        { a0=a; b0=b; }

  FHE_PK(const FheParameters& pms);

  FHE_PK(const FheParameters& pms, const Bigint& p)
    : a0(pms.FFTD(),evaluation,evaluation),
      b0(pms.FFTD(),evaluation,evaluation),
      sw_a(pms.FFTD(),evaluation,evaluation), 
      sw_b(pms.FFTD(),evaluation,evaluation) 
       { params=&pms; pr=p; }

  FHE_PK(const FheParameters& pms, int p) :
      FHE_PK(pms, Bigint(p))
  {
  }

  template<class FD>
  FHE_PK(const FheParameters& pms, const FD& FTD) :
      FHE_PK(pms, FTD.get_prime())
  {
  }

  // Rely on default copy constructor/assignment
  
  const RqElement& a() const { return a0; }
  const RqElement& b() const { return b0; }

  const RqElement& as() const { return sw_a; }
  const RqElement& bs() const { return sw_b; }

  
  // c must have same params as PK and rc
  template <class T, class FD, class S>
  void encrypt(Ciphertext& c, const Plaintext<T, FD, S>& mess, const RandomCoins& rc) const;

  template <class S>
  void encrypt(Ciphertext& c, const vector<S>& mess, const RandomCoins& rc) const;

  void quasi_encrypt(Ciphertext& c, const RqElement& mess, const RandomCoins& rc) const;

  template <class FD>
  Ciphertext encrypt(const Plaintext<typename FD::T, FD, typename FD::S>& mess, const RandomCoins& rc) const;

  /// Encryption
  template <class FD>
  Ciphertext encrypt(const Plaintext<typename FD::T, FD, typename FD::S>& mess) const;
  Ciphertext encrypt(const RqElement& mess) const;

  friend void key_gen(FHE_PK& PK,FHE_SK& SK,PRNG& G);

  RqElement sample_secret_key(PRNG& G);
  void key_gen(RqElement& sk, PRNG& G, int noise_boost = 1);
  void partial_key_gen(const RqElement& sk, const RqElement& a, PRNG& G,
      int noise_boost = 1);

  void check_noise(const FHE_SK& sk) const;
  void check_noise(const RqElement& x, bool check_modulo = false) const;

  /// Append to buffer
  void pack(OctetStream& o) const;

  /// Read from buffer. Assumes parameters are set correctly
  void unpack(OctetStream& o);
  
  bool operator!=(const FHE_PK& x) const;

  void check(const FheParameters& params, const Bigint& pr) const;
};


// PK and SK must have the same params, otherwise an error
void key_gen(FHE_PK& PK,FHE_SK& SK,PRNG& G);


/**
 * BGV key pair
 */
class FheKeyPair
{
public:
  /// Public key
  FHE_PK pk;
  /// Secret key
  FHE_SK sk;

  FheKeyPair(const FheParameters& params, const Bigint& pr) :
      pk(params, pr), sk(params, pr)
  {
  }

  /// Initialization
  FheKeyPair(const FheParameters& params) :
      pk(params), sk(params)
  {
  }

  void generate(PRNG& G)
  {
    key_gen(pk, sk, G);
  }

  /// Generate fresh keys
  void generate()
  {
    SeededPrng G;
    generate(G);
  }
};

template <class S>
void FHE_PK::encrypt(Ciphertext& c, const vector<S>& mess,
    const RandomCoins& rc) const
{
  RqElement mm((*params).FFTD(),polynomial,polynomial);
  mm.from(Iterator<S>(mess));
  quasi_encrypt(c, mm, rc);
}
