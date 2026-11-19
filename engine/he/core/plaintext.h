#pragma once

/* 
 * Defines plaintext either as a vector of field elements (gfp or Gf2nShort)
 * or as a vector of int (for Gf2nShort) or bigints (for gfp)
 *
 * The first format is the actual data we want, the second format
 * is the data we use to encrypt/decrypt. Passing between the two
 * depends on whether we have a gfp or a Gf2nShort base type.
 *   - The former uses the FFT (in the case of special m/p values
 *     and using FftData), or naive methods (for general m/p values
 *     and using PPData), the second uses a precomputed linear
 *     map
 *
 * We hold data however in vector format, as this is easier to 
 * deal with
 */

#include "he/core/generator.h"
#include "he/core/fft_data.h"
#include "numeric/fixint.h"

#include <vector>
using namespace std;

class FHE_PK;
class RqElement;
class FheParameters;
class FftData;
template<class T> class AddableVector;

// Forward declaration as apparently this is needed for friends in templates
template<class T,class FD,class S> class Plaintext;
template<class T,class FD,class S> void add(Plaintext<T,FD,S>& z,const Plaintext<T,FD,S>& x,const Plaintext<T,FD,S>& y);
template<class T,class FD,class S> void sub(Plaintext<T,FD,S>& z,const Plaintext<T,FD,S>& x,const Plaintext<T,FD,S>& y);
template<class T,class FD,class S> void mul(Plaintext<T,FD,S>& z,const Plaintext<T,FD,S>& x,const Plaintext<T,FD,S>& y);
template<class T,class FD,class S> void sqr(Plaintext<T,FD,S>& z,const Plaintext<T,FD,S>& x);

enum condition { Full, Diagonal, Bits };

enum PtType   { Polynomial, Evaluation, Both }; 

/**
 * BGV plaintext.
 * Use ``PlaintextModPrime`` instead of filling in the templates.
 * The plaintext is held in one of the two representations or both,
 * polynomial and evaluation. The latter is the one allowing element-wise
 * multiplication over a vector.
 * Plaintexts can be added, subtracted, and multiplied via operator overloading.
 */
template<class T,class FD,class _>
class Plaintext
{
  typedef typename FD::poly_type S;


  mutable vector<T> a;  // The thing in evaluation/FFT form
  mutable vector<S> b;  // Now in polynomial form

  mutable PtType type;

  /* We keep pointers to the basic data here 
   *   - FD is of type FftData or PPData if T is of type gfp
   *   - FD is of type P2Data if T is of type Gf2nShort
   */

  const FD   *field_data;

  int degree() const;

  public:
  
  const FD& get_field() const { return *field_data; }

  /// Number of slots
  unsigned int num_slots() const;

  Plaintext(const FD& field_d, PtType type = Polynomial)
  { field_data=&field_d; allocate(type); }

  Plaintext(const FD& field_d, const RqElement& other);

  /// Initialization
  Plaintext(const FheParameters& params);

  void allocate(PtType type) const;
  void allocate() const { allocate(type); }
  void allocate_slots(const Bigint& value);
  int get_min_alloc();

  /**
   * Read slot.
   * @param i slot number
   * @returns slot content
   */
  T element(int i) const
    {  if (type==Polynomial)  
         { from_polynomial(); }
       return a[i]; 
    }
  /**
   * Write to slot
   * @param i slot number
   * @param e new slot content
   */
  void set_element(int i,const T& e)
    { if (type==Polynomial)
        { throw NotImplemented(); }
      a.resize(num_slots());
      a[i]=e;
      type=Evaluation;
    }

  // Access poly representation
  const S& coeff(int i) const
    {  if (type!=Polynomial)
         { to_polynomial(); }
       return b[i];
    }

  void set_coefficient(int i,const S& e)
    {
      to_polynomial();
      type=Polynomial;
      b[i]=e;
    }

  const S& operator[](int i) const { return coeff(i); }

  // Assumes v is of the correct length (phi_m) for the given ring
  // and is already reduced modulo the correct prime etc
  void set_polynomial(const vector<S>& v)
    { type=Polynomial; b=v; }
  const vector<S>& get_polynomial() const
    {
      to_polynomial();
      return b;
    }

  Iterator<S> get_iterator() const { to_polynomial(); return b; }

  void from(const Generator<Bigint>& source) const;

  // This sets a poly from a vector of bigint's which needs centering
  // modulo mod, before assigning (used in decryption)
  //    vv[i] is already assumed reduced modulo mod though but in 
  //    range [0,...,mod)
  void set_polynomial_mod(const vector<Bigint>& vv,const Bigint& mod)
  {
    set_polynomial_mod(Iterator<Bigint>(vv), mod);
  }
  void set_polynomial_mod(const Generator<Bigint>& generator, const Bigint& mod);

  // Converts between Evaluation,Polynomial and Both representations
  //    Marked as const because does not change value, only changes the
  //    internal representation
  void from_polynomial() const;
  void to_polynomial() const;

  void randomize(PRNG& G,condition cond=Full);
  void randomize(PRNG& G, int n_bits, bool Diag=false, PtType type=Polynomial);

  void assign_zero(PtType t = Evaluation);
  void assign_one(PtType t = Evaluation);
  void assign_constant(T constant, PtType t = Evaluation);

  friend void add<>(Plaintext& z,const Plaintext& x,const Plaintext& y);
  friend void sub<>(Plaintext& z,const Plaintext& x,const Plaintext& y);
  friend void mul<>(Plaintext& z,const Plaintext& x,const Plaintext& y);
  friend void sqr<>(Plaintext& z,const Plaintext& x);

  Plaintext operator+(const Plaintext& x) const
  { Plaintext res(*field_data); add(res, *this, x); return res; }
  Plaintext operator-(const Plaintext& x) const
  { Plaintext res(*field_data); sub(res, *this, x); return res; }

  void mul(const Plaintext& x, const Plaintext& y)
  { x.from_polynomial(); y.from_polynomial(); ::mul(*this, x, y); }

  Plaintext operator*(const Plaintext& x)
  { Plaintext res(*field_data); res.mul(*this, x); return res; }

  Plaintext& operator+=(const Plaintext& y);
  Plaintext& operator-=(const Plaintext& y)
  { to_polynomial(); y.to_polynomial(); ::sub(*this, *this, y); return *this; }

  void negate();

  AddableVector<S> mult_by_x_i(int i, const FHE_PK& pk) const;

  bool equals(const Plaintext& x) const;
  bool operator!=(const Plaintext& x) { return !equals(x); }

  bool is_diagonal() const;

  /// Append to buffer
  void pack(OctetStream& o) const;

  /// Read from buffer. Assumes parameters are set correctly
  void unpack(OctetStream& o);

  size_t report_size(ReportType type);

  void print_evaluation(int n_elements, string desc = "") const;
};

template <class FD>
using Plaintext_ = Plaintext<typename FD::T, FD, typename FD::S>;

typedef Plaintext_<FftData> PlaintextModPrime;
