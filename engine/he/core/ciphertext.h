#pragma once

#include "he/core/fhe_keys.h"
#include "he/core/random_coins.h"
#include "he/core/plaintext.h"

class FHE_PK;
class Ciphertext;

// Forward declare the friend functions
template<class T,class FD,class S> void mul(Ciphertext& ans,const Plaintext<T,FD,S>& a,const Ciphertext& c);
template<class T,class FD,class S> void mul(Ciphertext& ans,const Ciphertext& c,const Plaintext<T,FD,S>& a);

void add(Ciphertext& ans,const Ciphertext& c0,const Ciphertext& c1);
void mul(Ciphertext& ans,const Ciphertext& c0,const Ciphertext& c1,const FHE_PK& pk);

/**
 * BGV ciphertext.
 * The class allows adding two ciphertexts as well as adding a plaintext and
 * a ciphertext via operator overloading. The multiplication of two ciphertexts
 * requires the public key and thus needs a separate function.
 */
class Ciphertext
{
  RqElement cc0,cc1;
  const FheParameters *params;
  // identifier for debugging
  word pk_id;

  public:

  static const false_type optimized_packing;

  const FheParameters& get_parameters() const { return *params; }

  Ciphertext(const FheParameters& p)
    : cc0(p.FFTD(),evaluation,evaluation),
      cc1(p.FFTD(),evaluation,evaluation), pk_id(0)  { params=&p; }

  Ciphertext(const FHE_PK &pk);

  Ciphertext(const RqElement& a0, const RqElement& a1, const Ciphertext& C) :
      Ciphertext(C.get_parameters())
  {
    set(a0, a1, C.get_pk_id());
  }

  // Rely on default copy assignment/constructor
  
  word get_pk_id() const { return pk_id; }

  void set(const RqElement& a0, const RqElement& a1, word pk_id)
    { cc0=a0; cc1=a1; this->pk_id = pk_id; }
  void set(const RqElement& a0, const RqElement& a1, const FHE_PK& pk);

  const RqElement& c0() const { return cc0; }
  const RqElement& c1() const { return cc1; }
  
  void assign_zero() { cc0.assign_zero(); cc1.assign_zero(); pk_id = 0; }

  // Scale down an element from level 1 to level 0, if at level 0 do nothing
  void Scale(const Bigint& p)    { cc0.Scale(p); cc1.Scale(p); }
  void Scale();

  // Throws error if ans,c0,c1 etc have different params settings
  //   - Thus programmer needs to ensure this rather than this being done
  //     automatically. This saves some time in space initialization
  friend void add(Ciphertext& ans,const Ciphertext& c0,const Ciphertext& c1);
  friend void sub(Ciphertext& ans,const Ciphertext& c0,const Ciphertext& c1);
  friend void mul(Ciphertext& ans,const Ciphertext& c0,const Ciphertext& c1,const FHE_PK& pk);
  template<class T,class FD,class S> friend void mul(Ciphertext& ans,const Plaintext<T,FD,S>& a,const Ciphertext& c);
  template<class T,class FD,class S> friend void mul(Ciphertext& ans,const Ciphertext& c,const Plaintext<T,FD,S>& a)
     { ::mul(ans,a,c); }

  void mul(const Ciphertext& c, const RqElement& a);

  template<class FD>
  void mul(const Ciphertext& c, const Plaintext_<FD>& a) { ::mul(*this, c, a); }

  template<class FD>
  Ciphertext operator+(const Plaintext_<FD>& other) { Ciphertext res = *this; res += other; return res; }
  template<class FD>
  Ciphertext& operator+=(const Plaintext_<FD>& other) { cc0 += other.get_polynomial(); return *this; }

  bool operator==(const Ciphertext& c) const { return pk_id == c.pk_id && cc0.equals(c.cc0) && cc1.equals(c.cc1); }
  bool operator!=(const Ciphertext& c) const { return !(*this == c); }

  Ciphertext operator+(const Ciphertext& other) const
  { Ciphertext res(*params); ::add(res, *this, other); return res; }

  template <class FD>
  Ciphertext operator*(const Plaintext_<FD>& other) const
  { Ciphertext res(*params); ::mul(res, *this, other); return res; }

  Ciphertext& operator+=(const Ciphertext& other) { ::add(*this, *this, other); return *this; }

  template <class FD>
  Ciphertext& operator*=(const Plaintext_<FD>& other) { ::mul(*this, *this, other); return *this; }

  /**
   * Ciphertext multiplication.
   * @param pk public key
   * @param x second ciphertext
   * @returns product ciphertext
   */
  Ciphertext mul(const FHE_PK& pk, const Ciphertext& x) const
  { Ciphertext res(*params); ::mul(res, *this, x, pk); return res; }

  Ciphertext mult_by_x_i(int i, const FHE_PK&) const
  {
    return {cc0.mult_by_x_i(i), cc1.mult_by_x_i(i), *this};
  }

  /// Re-randomize for circuit privacy.
  void rerandomize(const FHE_PK& pk);

  int level() const { return cc0.level(); }

  /// Append to buffer
  void pack(OctetStream& o, int = -1) const
    { cc0.pack(o); cc1.pack(o); o.store(pk_id); }

  /// Read from buffer. Assumes parameters are set correctly
  void unpack(OctetStream& o, int = -1)
    { cc0.unpack(o, *params); cc1.unpack(o, *params); o.get(pk_id); }

  void output(ostream& s) const
    { cc0.output(s); cc1.output(s); s.write((char*)&pk_id, sizeof(pk_id)); }
  void input(istream& s)
    { cc0.input(s); cc1.input(s); s.read((char*)&pk_id, sizeof(pk_id)); }

  void add(OctetStream& os, int = -1);

  size_t report_size(ReportType type) const { return cc0.report_size(type) + cc1.report_size(type); }
};
