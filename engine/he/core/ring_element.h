#pragma once

/* Defines an element of the ring modulo a prime pr
 *   - Note here pr is an odd prime
 * We also assume that pr splits completely over the underlying
 * ring. Equivalently (as the ring is of m'th roots of unity),
 * we have that pr-1 is divisible by m.
 *   - If m is not a power of two we also require pr-1 is divisible by 
 *     some power of two, this is to get Bluestein's FFT working
 * Thus we can define both a polynomial and an evaluation 
 * representation
 */

enum RepType { polynomial, evaluation };

#include "he/core/fft_data.h"
#include "support/octet_stream.h"
#include "support/random.h"
#include "he/core/generator.h"
#include <iostream>
#include <vector>
using namespace std;

class RingWriteIterator;
class RingReadIterator;

class RingElement
{
  friend class RqElement;

  RepType rep;

  /* FFTD is defined as a pointer so each different RingElement
   * can be wrt a different prime if need be
   *   - Recall FFTD also contains data about the Ring
   */
  const FftData *FFTD;  

  /* In either representation we hold the element as an array of
   * modp's of length Ring.phi_m()
   */

  vector<modp> element; 

  /* Careful calling this one, as FFTD will not be defined */
  RingElement(RepType r=polynomial) : FFTD(0) { rep=r; }

  public:

  // Used to basically make sure *this is able to cope
  // with being assigned to by something of "type" e
  void partial_assign(const RingElement& e)
    { rep=e.rep; FFTD=e.FFTD; 
      if (FFTD)
        element.resize((*FFTD).phi_m());
    }

  void prepare(const RingElement& e);
  void prepare_push();
  void allocate();

  void set_data(const FftData& prd)   { FFTD=&prd; }
  const FftData& get_fftd() const     { assert(FFTD); return *FFTD; }
  const ZpData& get_pr_d() const   { return get_fftd().get_pr_d(); }
  const Bigint&  get_prime() const { return get_fftd().get_prime(); }

  void assign_zero();
  void assign_one();

  RingElement(const FftData& prd,RepType r=polynomial);

  template<class T>
  RingElement(const FftData& prd, RepType r, const vector<T>& other)
    {
      assert(size_t(prd.num_slots()) == other.size());
      FFTD = &prd;
      rep = r;
      for (auto& x : other)
        element.push_back({x, FFTD->get_pr_d()});
    }

  /* Functional Operators */
  void negate();
  friend void add(RingElement& ans,const RingElement& a,const RingElement& b);
  friend void sub(RingElement& ans,const RingElement& a,const RingElement& b);
  friend void mul(RingElement& ans,const RingElement& a,const RingElement& b);
  friend void mul(RingElement& ans,const RingElement& a,const modp& b);

  RingElement mult_by_x_i(int i) const;

  RingElement& operator+=(const RingElement& other);
  RingElement& operator-=(const RingElement& other);
  RingElement& operator*=(const RingElement& other);
  RingElement& operator*=(const modp& other);

  void randomize(PRNG& G,bool Diag=false);

  bool equals(const RingElement& a) const;
  bool is_zero() const;

  // This is a NOP in cases where we cannot do a FFT
  void change_rep(RepType r);

  // Converting to and from a vector of bigint/int's 
  // I/O is assumed to be in poly rep, so from_vec it internally alters
  // the representation to the current representation
  vector<Bigint>  to_vector_bigint() const;
  void to_vector_bigint(vector<Bigint>& v) const;

  ConversionIterator get_iterator() const;

  friend class RingReadIterator;
  RingReadIterator get_copy_iterator() const;

  friend class RingWriteIterator;
  RingWriteIterator get_write_iterator();

  template <class T>
  void from(const Generator<T>& generator);

  template <class T>
  void from(const vector<T>& source)
  {
    assert(source.size() == (size_t) get_fftd().phi_m());
    from(Iterator<T>(source));
  }

  // This gets the constant term of the poly rep as a modp element
  modp get_constant() const;
  modp get_element(int i) const
  {
    if (element.empty())
      return {};
    else
      return element[i];
  }
  void set_element(int i,const modp& a)
  {
    allocate();
    element[i] = a;
  }

  /* Pack and unpack into an OctetStream 
   *   For unpack we assume the FFTD has been assigned correctly already
   */
  void pack(OctetStream& o) const;
  void unpack(OctetStream& o);

  void check_rep();
  void check_size() const;

  void output(ostream& s) const;
  void input(istream& s);

  void check(const FftData& FFTD) const;

  size_t report_size(ReportType type) const;
};


class RingWriteIterator : public WriteConversionIterator
{
  RingElement& element;
  RepType rep;
public:
  RingWriteIterator(RingElement& element) :
    WriteConversionIterator(element.element, element.get_fftd().get_pr_d()),
    element(element), rep(element.rep)
  {
    element.rep = polynomial;
    element.allocate();
  }
  ~RingWriteIterator() { element.change_rep(rep); }
};


class RingReadIterator : public ConversionIterator
{
  RingElement element;
public:
  RingReadIterator(const RingElement& element) :
    ConversionIterator(this->element.element, element.get_fftd().get_pr_d()),
    element(element)
  {
    this->element.change_rep(polynomial);
    this->element.allocate();
  }
};


inline void mul(RingElement& ans,const modp& a,const RingElement& b)
{ mul(ans,b,a); }


template <class T>
void RingElement::from(const Generator<T>& generator)
{
  RepType t=rep;
  rep=polynomial;
  T tmp;
  modp tmp2;
  prepare_push();
  assert(FFTD);
  for (int i=0; i<(*FFTD).phi_m(); i++)
    {
      generator.get(tmp);
      tmp2.convert_destroy(tmp, (*FFTD).get_pr_d());
      element.push_back(tmp2);
    }
  change_rep(t);
}
