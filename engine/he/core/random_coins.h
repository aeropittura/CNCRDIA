#pragma once

/*  Randomness used to encrypt */

#include "he/core/fhe_params.h"
#include "he/core/rq_element.h"
#include "he/core/addable_vector.h"

class FHE_PK;

#ifndef N_LIMBS_RAND
#define N_LIMBS_RAND 1
#endif

class IntRandomCoins : public AddableMatrix<Fixint<N_LIMBS_RAND>>
{
  typedef value_type::value_type T;

  const FheParameters* params;
public:
  typedef value_type::value_type rand_type;

  IntRandomCoins(const FheParameters& params) : params(&params)
  { resize(3, params.phi_m()); }

  IntRandomCoins(const FHE_PK& pk);

  void sample(PRNG& G)
  {
    (*this)[0].from(HalfGenerator<T>(G));
    for (int i = 1; i < 3; i++)
      (*this)[i].from(GaussianGenerator<T>(params->get_dg(), G));
  }
};

class RandomCoins
{
  typedef Bigint T;

  RqElement uu,vv,ww;
  const FheParameters *params;

  public:

  const FheParameters& get_parameters() const { return *params; }

  RandomCoins(const FheParameters& p) 
    : uu(p.FFTD(),evaluation,evaluation),
      vv(p.FFTD(),evaluation,evaluation),
      ww(p.FFTD(),polynomial,polynomial)
      { params=&p; }

  RandomCoins(const FHE_PK& pk);
  
  // Rely on default copy assignment/constructor

  const RqElement& u() const { return uu; }
  const RqElement& v() const { return vv; }
  const RqElement& w() const { return ww; }

  void assign(const RqElement& u,const RqElement& v,const RqElement& w)
    { uu=u; vv=v; ww=w; }

  template <class T>
  void assign(const vector<T>& u,const vector<T>& v,const vector<T>& w)
    {
      uu.from(u);
      vv.from(v);
      ww.from(w);
    }

  void assign(const IntRandomCoins& rc)
    {
      uu.from(rc[0]);
      vv.from(rc[1]);
      ww.from(rc[2]);
    }

  /* Generate a standard distribution */
  void generate(PRNG& G)
    {
      uu.from(HalfGenerator<T>(G));
      vv.from(GaussianGenerator<T>(params->get_dg(), G));
      ww.from(GaussianGenerator<T>(params->get_dg(), G));
    }

  // Generate all from Uniform in range (-B,...B)
  void generate_uniform(PRNG& G,const Bigint& B1,const Bigint& B2,const Bigint& B3)
    {
      if (B1 == 0)
        uu.assign_zero();
      else
        uu.from(UniformGenerator<T>(G,num_bits(B1)));
      vv.from(UniformGenerator<T>(G,num_bits(B2)));
      ww.from(UniformGenerator<T>(G,num_bits(B3)));
    }


  // ans,x and y must have same params otherwise error
  friend void add(RandomCoins& ans,
                  const RandomCoins& x,const RandomCoins& y);

  void pack(OctetStream& o) const { uu.pack(o); vv.pack(o); ww.pack(o); }

  size_t report_size(ReportType type)
  { return uu.report_size(type) + vv.report_size(type) + ww.report_size(type); }
};
