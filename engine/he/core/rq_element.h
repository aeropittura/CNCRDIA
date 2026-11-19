#pragma once

/* An Rq Element is something held modulo Q_0 = p0 or Q_1 = p0*p1
 *
 * The level is the value of Q_level which is being used.
 * Elements can be held in either representation and one can switch
 * representations at will.
 *   - Although in the evaluation we do not multiply at level 1
 *     we do need to multiply at level 1 for key_gen and Encryption.
 *
 * Usually we keep level 0 in evaluation and level 1 in polynomial
 * representation though
 */

#include "he/core/ring_element.h"
#include "he/core/fhe_params.h"
#include "he/core/tools.h"
#include "he/core/generator.h"
#include "he/core/plaintext.h"
#include <vector>

// Forward declare the friend functions
class RqElement;
void add(RqElement& ans,const RqElement& a,const RqElement& b);
void sub(RqElement& ans,const RqElement& a,const RqElement& b);
void mul(RqElement& ans,const RqElement& a,const RqElement& b);
void mul(RqElement& ans,const RqElement& a,const Bigint& b);

class RqElement
{
protected:
  vector<RingElement> a;
  int lev;

  // Must be careful not to call by mistake
  RqElement(RepType r0=evaluation,RepType r1=polynomial) :
    a({r0, r1}), lev(n_mults()) {}

  public:

  static false_type optimized_packing;
  
  int n_mults() const { return a.size() - 1; }

  void change_rep(RepType r);
  void change_rep(RepType r0,RepType r1);

  void set_data(const vector<FftData>& prd);
  void assign_zero(const vector<FftData>& prd);
  void assign_zero(); 
  void assign_one(); 
  void partial_assign(const RqElement& e);

  // Pass in a pair of FftData as a vector
  RqElement(const vector<FftData>& prd, RepType r0 = evaluation,
      RepType r1 = polynomial);

  RqElement(const FheParameters& params, RepType r0, RepType r1) :
      RqElement(params.FFTD(), r0, r1) {}

  RqElement(const FheParameters& params) :
      RqElement(params.FFTD()) {}

  RqElement(const FHE_PK& pk);

  RqElement(const RingElement& b0,const RingElement& b1) :
    a({b0, b1}), lev(n_mults())
  {
    assert(b0.get_fftd().get_R() == b1.get_fftd().get_R());
  }

  RqElement(const RingElement& b0) :
    a({b0}), lev(n_mults()) {}

  template<class T, class FD, class S>
  RqElement(const FheParameters& params, const Plaintext<T, FD, S>& plaintext,
      RepType r0 = polynomial, RepType r1 = polynomial) :
      RqElement(params, r0, r1)
  {
    from(plaintext.get_iterator());
  }

  template<class U, class V>
  RqElement(const vector<FftData>& prd, const vector<U>& b0,
      const vector<V>& b1, RepType r = evaluation) :
      RqElement(prd, r, r)
  {
    a[0] = RingElement(prd[0], r, b0);
    a[1] = RingElement(prd[1], r, b1);
  }

  const RingElement& get(int i) const { return a[i]; }

  /* Functional Operators */
  void negate();
  friend void add(RqElement& ans,const RqElement& a,const RqElement& b);
  friend void sub(RqElement& ans,const RqElement& a,const RqElement& b);
  friend void mul(RqElement& ans,const RqElement& a,const RqElement& b);
  friend void mul(RqElement& ans,const RqElement& a,const Bigint& b);

  template<class S>
  RqElement& operator+=(const vector<S>& other);

  RqElement& operator+=(const RqElement& other) { ::add(*this, *this, other); return *this; }

  RqElement operator+(const RqElement& b) const { RqElement res(*this); ::add(res, *this, b); return res; }
  RqElement operator-(const RqElement& b) const { RqElement res(*this); sub(res, *this, b); return res; }
  template <class T>
  RqElement operator*(const T& b) const { RqElement res(*this); mul(res, *this, b); return res; }

  // Multiply something by p1 and make level 1
  void mult_by_p1();

  RqElement mult_by_x_i(int i) const;

  void randomize(PRNG& G,int lev=-1);

  // Scale from level 1 to level 0, if at level 0 do nothing
  void Scale(const Bigint& p);

  bool equals(const RqElement& a) const;
  bool operator==(const RqElement& a) const { return equals(a); }
  bool operator!=(const RqElement& a) const { return !equals(a); }

  int level() const { return lev; }
  void lower_level() { if (lev==1) { lev=0; }  }
  // raise_level boosts a level 0 to a level 1 (or does nothing if level =1)
  void raise_level();
  void check_level() const;
  void set_level(int level) { lev = (level == -1 ? n_mults() : level); }
  void partial_assign(const RqElement& a, const RqElement& b);

  // Converting to and from a vector of bigint's Again I/O is in poly rep
  vector<Bigint>  to_vector_bigint() const;
  void to_vector_bigint(vector<Bigint>& v) const;

  ConversionIterator get_iterator() const;
  template <class T>
  void from(const Generator<T>& generator, int level=-1);

  template <class T>
  void from(const vector<T>& source, int level=-1)
    {
      for (auto& x : a)
        assert(source.size() == (size_t) x.get_fftd().phi_m());
      from(Iterator<T>(source), level);
    }

  Bigint infinity_norm() const;

  Bigint get_prime(int i) const
    { return a[i].get_prime(); }

  Bigint get_modulus() const
    { Bigint ans=1;
      for(int i=0; i<=lev; ++i) ans*=a[i].get_prime();
      return ans;
    }

  /* Pack and unpack into an OctetStream 
   *   For unpack we assume the pr_data for a0 and a1 has been assigned 
   *   correctly already
   */
  void pack(OctetStream& o, int = -1) const;
  void unpack(OctetStream& o, int = -1);

  // without prior initialization
  void unpack(OctetStream& o, const FheParameters& params);

  void output(ostream& s) const;
  void input(istream& s);

  void check(const FheParameters& params) const;

  size_t report_size(ReportType type) const;

  void print_first_non_zero() const;
};
   
template<int L>
inline void mul(RqElement& ans,const Bigint& a,const RqElement& b)
{ mul(ans,b,a); }

template<class S>
RqElement& RqElement::operator+=(const vector<S>& other)
{
  RqElement tmp = *this;
  tmp.from(Iterator<S>(other), lev);
  ::add(*this, *this, tmp);
  return *this;
}

template <class T>
void RqElement::from(const Generator<T>& generator, int level)
{
  set_level(level);
  if (lev == 1)
    {
      auto clone = generator.clone();
      a[1].from(*clone);
      delete clone;
    }
  a[0].from(generator);
}
