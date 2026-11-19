/*
 * he/offline/producer.h
 *
 */

#pragma once

#include "net/player.h"
#include "he/core/plaintext.h"
#include "he/offline/enc_commit.h"
#include "he/offline/dist_decrypt.h"
#include "he/offline/sacrificing.h"
#include "protocols/common/share.h"
#include "numeric/setup.h"

template <class T>
string prep_filename(string type, int my_num, int thread_num,
    bool initial, string dir = PREP_DIR);
template <class T>
string open_prep_file(ofstream& outf, string type, int my_num, int thread_num,
    bool initial, bool clear, string dir = PREP_DIR);

template <class FD>
class Producer
{
protected:
  int n_slots;
  int output_thread;
  bool write_output;
  string dir;

public:
  typedef typename FD::T T;
  typedef typename FD::S S;

  map<string, Timer> timers;

  static void remove_secrets(const char* phase);

  Producer(int output_thread = 0, bool write_output = true);
  virtual ~Producer() {}
  virtual string data_type() = 0;
  virtual condition required_randomness() { return Full; }

  virtual string open_file(ofstream& outf, int my_num, int thread_num, bool initial,
      bool clear);
  virtual void clear_file(int my_num, int thread_num = 0, bool initial = true);

  virtual void clear() = 0;

  void run_with_catch(const Player& P, const FHE_PK& pk,
      const Ciphertext& calpha, EncCommitBase<T, FD, S>& EC,
      DistDecrypt<FD>& dd, const T& alphai);
  int sacrifice_with_catch(const Player& P, MacCheck<T>& MC);

  virtual void run(const Player& P, const FHE_PK& pk,
      const Ciphertext& calpha, EncCommitBase<T, FD, S>& EC,
      DistDecrypt<FD>& dd, const T& alphai) = 0;
  virtual int sacrifice(const Player& P, MacCheck<T>& MC) = 0;

  int num_slots() { return n_slots; }

  virtual size_t report_size(ReportType type) { (void)type; return 0; }
};

template <class T, class FD, class S>
class TripleProducer : public TripleSacriFactory< Share<T> >, public Producer<FD>
{
  unsigned int i;

public:
  Plaintext_<FD> values[3], macs[3];
  Plaintext<T,FD,S> &ai, &bi, &ci;
  Plaintext<T,FD,S> &gam_ai, &gam_bi, &gam_ci;

  string data_type() { return "Triples"; }

  TripleProducer(const FD& Field, int my_num, int output_thread = 0,
      bool write_output = true, string dir = PREP_DIR);

  void clear() { this->triples.clear(); }
  void run(const Player& P, const FHE_PK& pk,
      const Ciphertext& calpha, EncCommitBase<T, FD, S>& EC,
      DistDecrypt<FD>& dd, const T& alphai);

  int sacrifice(const Player& P, MacCheck<T>& MC);
  void get(Share<T>& a, Share<T>& b, Share<T>& c);
  void reset() { i = 0; }

  int num_slots() { return ai.num_slots(); }

  size_t report_size(ReportType type);
};

template <class FD>
using TripleProducer_ = TripleProducer<typename FD::T, FD, typename FD::S>;

template <class FD>
class TupleProducer : public Producer<FD>
{
protected:
  typedef typename FD::T T;

  unsigned int i;

public:
  Plaintext_<FD> values[2], macs[2];

  TupleProducer(const FD& field_d, int output_thread = 0,
      bool write_output = true);
  virtual ~TupleProducer() {}

  void compute_macs(const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, Ciphertext& ca,
      Ciphertext & cb);

  virtual void get(Share<T>& a, Share<T>& b);
};

template <class FD>
class InverseProducer : public TupleProducer<FD>,
    public TupleSacriFactory< Share<typename FD::T> >
{
  typedef typename FD::T T;

  Plaintext_<FD> ab;

  TripleProducer_<FD> triple_producer;
  bool produce_triples;

public:
  Plaintext_<FD> &ai, &bi;
  Plaintext_<FD> &gam_ai, &gam_bi;

  InverseProducer(const FD& field_d, int my_num, int output_thread = 0,
      bool write_output = true, bool produce_triples = true,
      string dir = PREP_DIR);
  string data_type() { return "Inverses"; }

  void clear() { this->tuples.clear(); }
  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha, 
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai);
  void get(Share<T>& a, Share<T>& b);
  int sacrifice(const Player& P, MacCheck<T>& MC);
};

template <class FD>
class SquareProducer : public TupleProducer<FD>,
    public TupleSacriFactory< Share<typename FD::T> >
{
public:
  typedef typename FD::T T;

  Plaintext_<FD> &ai, &bi;
  Plaintext_<FD> &gam_ai, &gam_bi;

  SquareProducer(const FD& field_d, int my_num, int output_thread = 0,
      bool write_output = true, string dir = PREP_DIR);
  string data_type() { return "Squares"; }

  void clear() { this->tuples.clear(); }
  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai);
  void get(Share<T>& a, Share<T>& b) { TupleProducer<FD>::get(a, b); }
  int sacrifice(const Player& P, MacCheck<T>& MC);
};

class GfpBitProducer : public Producer<FftData>,
    public SingleSacriFactory< Share<Gfp> >
{
  typedef FftData FD;

  unsigned int i;
  Plaintext_<FD> vi, gam_vi;

  SquareProducer<FftData> square_producer;
  bool produce_squares;

public:
  vector< Share<Gfp> > bits;

  GfpBitProducer(const FD& field_d, int my_num, int output_thread = 0,
      bool write_output = true, bool produce_squares = true,
      string dir = PREP_DIR);
  string data_type() { return "Bits"; }

  void clear() { bits.clear(); }
  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const Gfp& alphai);
  int output(const Player& P, int thread, int output_thread = 0);

  void get(Share<Gfp>& a);
  int sacrifice(const Player& P, MacCheck<T>& MC);
};

// glue
template<class FD>
Producer<FD>* new_bit_producer(const FD& field_d, const Player& P,
    const FHE_PK& pk, int covert,
    bool produce_squares = true, int thread_num = 0, bool write_output = true,
    string dir = PREP_DIR);

class Gf2nBitProducer : public Producer<P2Data>
{
  typedef P2Data FD;
  typedef Gf2nShort T;

  ofstream outf;
  bool write_output;

  EncCommit_<FD> ECB;

public:
  Gf2nBitProducer(const Player& P, const FHE_PK& pk, int covert,
      int output_thread = 0, bool write_output = true, string dir = PREP_DIR);
  string data_type() { return "Bits"; }

  void clear() {}
  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai);

  int sacrifice(const Player& P, MacCheck<T>& MC);
};

template <class FD>
class InputProducer : public Producer<FD>
{
  typedef typename FD::T T;
  typedef typename FD::S S;

  ofstream* outf;
  const Player& P;
  bool write_output;

public:
  vector<vector<InputTuple<Share<T>>>> inputs;

  InputProducer(const Player& P, int output_thread = 0, bool write_output = true,
      string dir = PREP_DIR);
  ~InputProducer();

  string data_type() { return "Inputs"; }

  void clear() {}

  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai)
  {
      run(P, pk, calpha, EC, dd, alphai, -1);
  }

  void run(const Player& P, const FHE_PK& pk, const Ciphertext& calpha,
      EncCommitBase_<FD>& EC, DistDecrypt<FD>& dd, const T& alphai,
      int player);

  int sacrifice(const Player& P, MacCheck<T>& MC);

  // no ops
  string open_file(ofstream& outf, int my_num, int thread_num, bool initial,
      bool clear);
  void clear_file(int my_num, int thread_num = 0, bool initial = 0);

};
