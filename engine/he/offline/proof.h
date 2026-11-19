#pragma once

#include <math.h>
#include <vector>
using namespace std;

#include "numeric/bigint.h"
#include "he/core/ciphertext.h"
#include "he/core/addable_vector.h"
#include "protocols/spdz/cowgear_options.h"
#include "vm/online_options.h"

#include "he/offline/config.h"

enum SlackType
{
  NONINTERACTIVE_SPDZ1_SLACK = -1,
  INTERACTIVE_SPDZ1_SLACK = -2,
  COVERT_SPDZ2_SLACK = -3,
  ACTIVE_SPDZ2_SLACK = -4,
};

class Proof
{
  protected:

  unsigned int sec;

  bool diagonal;

  OctetStream session_id;

  Proof();   // Private to avoid default 

  public:

  typedef AddableVector< IntRandomCoins > Randomness;
  typedef typename FftData::poly_type BoundType;

  class Preimages
  {
    typedef IntRandomCoins::value_type::value_type RType;

    BoundType m_temp;
    AddableVector<RType> r_temp;

  public:
    Preimages(int size, const FHE_PK& pk, const Bigint& p, int n_players);
    AddableMatrix<BoundType> m;
    Randomness r;
    void add(OctetStream& os);
    void pack(OctetStream& os);
    void unpack(OctetStream& os);
    void check_sizes();
    size_t report_size(ReportType type) { return m.report_size(type) + r.report_size(type); }
  };

  Bigint tau,rho;

  unsigned int phim;
  int b_plain_length, b_random_length;
  Bigint plain_check, random_check;
  unsigned int V;
  unsigned int U;

  const FHE_PK* pk;

  int n_proofs;

  vector<int> e;
  vector<vector<int>> W;
  bool top_gear;

  static double dist;

  protected:
    typedef AddableVector<BoundType> T;
    typedef AddableMatrix<typename IntRandomCoins::rand_type> X;

    Proof(int sc, const Bigint& Tau, const Bigint& Rho, const FHE_PK& pk,
            int n_proofs = 1, bool diagonal = false) :
              diagonal(diagonal),
              b_plain_length(0), b_random_length(0), pk(&pk), n_proofs(n_proofs)
    { sec=sc;
      assert(sec > 0);
      tau=Tau; rho=Rho;

      phim=(pk.get_parameters()).phi_m();

      top_gear = use_top_gear(pk);
      if (top_gear)
        {
          V = ceil((sec + 2) / log2(2 * phim + 1));
          U = 2 * V;
        }
      else
        {
          U = sec;
          V = 2 * sec - 1;
        }
    }

  Proof(int sec, const FHE_PK& pk, int n_proofs = 1, bool diagonal = false) :
      Proof(sec, pk.p() / 2,
          pk.get_parameters().get_dg().get_new_hope_b(), pk,
          n_proofs, diagonal) {}

  virtual ~Proof() {}

  public:
  static Bigint slack(int slack, int sec, int phim);

  bool use_top_gear(const FHE_PK& pk)
  {
    return CowGearOptions::singleton.top_gear() and pk.p() > 2 and
        not diagonal;
  }

  bool get_diagonal() const
  {
    return diagonal;
  }

  static int n_ciphertext_per_proof(int sec, const FHE_PK& pk, bool diagonal =
      false)
  {
    return Proof(sec, pk, 1, diagonal).U;
  }

  void set_session_id(const PlayerBase& P);
  void set_challenge(const OctetStream& ciphertexts, int prover,
      const FHE_PK& pk);
  void set_challenge(PRNG& G);
  void generate_challenge(const Player& P);

  bool check_bounds(T& z, X& t, int i) const;

  template<class T, class U>
  void apply_challenge(int i, T& output, const U& input, const FHE_PK& pk) const
  {
    if (top_gear)
      {
        for (unsigned j = 0; j < this->U; j++)
          if (W[i][j] >= 0)
            output += input[j].mult_by_x_i(W[i][j], pk);
      }
    else
      for (unsigned k = 0; k < sec; k++)
        {
          unsigned j = (i + 1) - (k + 1);
          if (j < sec && e.at(j))
            output += input.at(j);
        }
  }

  void debugging()
  {
    if (OnlineOptions::singleton.has_option("verbose_he"))
      {
        cerr << "Using " << U << " ciphertexts per proof" << endl;
        cerr << "Plaintext bound check bit length: " << b_plain_length << endl;
        cerr << "Randomness bound check bit length: " << b_random_length << endl;
      }
  }
};

class NonInteractiveProof : public Proof
{
public:
  // sec = 0 used for protocols without proofs
  static int comp_sec(int sec);

  Bigint static slack(int sec, int phim)
  { sec = comp_sec(sec); return Bigint(phim * sec * sec) << (sec / 2 + 8); }

  NonInteractiveProof(int sec, const FHE_PK& pk,
      int extra_slack, bool diagonal = false) :
      Proof(comp_sec(sec), pk, 1, diagonal)
  {
    sec = this->sec;
    Bigint B;
    B=128*sec*sec;
    B <<= extra_slack;
    b_plain_length = num_bits(B*phim*tau);
    b_random_length = num_bits(B*3*phim*rho);
    plain_check = (Bigint(1) << b_plain_length) - sec * tau;
    random_check = (Bigint(1) << b_random_length) - sec * rho;
    debugging();
  }
};

class InteractiveProof : public Proof
{
public:
  Bigint static slack(int sec, int phim)
  { (void)phim; return pow(2, 1.5 * sec + 1); }

  InteractiveProof(int sec, const FHE_PK& pk,
      int n_proofs = 1, bool diagonal = false) :
      Proof(sec, pk, n_proofs, diagonal)
  {
    Bigint B;
    B = Bigint(1) << sec;
    b_plain_length = num_bits(B * tau);
    b_random_length = num_bits(B * rho);
    // leeway for completeness
    plain_check = (Bigint(2) << b_plain_length);
    random_check = (Bigint(2) << b_random_length);
    debugging();
  }
};
