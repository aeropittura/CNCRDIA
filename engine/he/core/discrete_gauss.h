#pragma once

/* Class to sample from a Discrete Gauss distribution of 
   standard deviation R
*/

#include "he/core/generator.h"
#include "support/random.h"
#include <vector>
#include <math.h>

class DiscreteGauss
{
  /* This is the bound we use on for the NewHope approximation
   * to a discrete Gaussian with sigma=sqrt(B/2)
   */
  int new_hope_b;

  public:

  void set(double R);

  void pack(OctetStream& o) const { o.serialize(new_hope_b); }
  void unpack(OctetStream& o) { o.unserialize(new_hope_b); }

  DiscreteGauss(double R) { set(R); }

  // Rely on default copy constructor/assignment
  
  int sample(PRNG& G, int stretch = 1) const;
  double get_R() const { return sqrt(0.5 * new_hope_b); }
  int get_new_hope_b() const { return new_hope_b; }

  bool operator!=(const DiscreteGauss& other) const;
};

template<class T>
class RandomGenerator : public Generator<T>
{
protected:
  mutable PRNG G;

public:
  RandomGenerator(PRNG& G) { this->G.SetSeed(G); }
};

template<class T>
class UniformGenerator : public RandomGenerator<T>
{
  int n_bits;
  bool positive;

public:
  UniformGenerator(PRNG& G, int n_bits, bool positive = true) :
    RandomGenerator<T>(G), n_bits(n_bits), positive(positive) {}
  Generator<T>* clone() const { return new UniformGenerator<T>(*this); }
  void get(T& x) const  { this->G.get(x, n_bits, positive); }
};

template<class T = Bigint>
class GaussianGenerator : public RandomGenerator<T>
{
  DiscreteGauss DG;
  int stretch;

public:
  GaussianGenerator(const DiscreteGauss& DG, PRNG& G, int stretch = 1) :
    RandomGenerator<T>(G), DG(DG), stretch(stretch) {}
  Generator<T>* clone() const { return new GaussianGenerator<T>(*this); }
  void get(T& x) const { x = DG.sample(this->G, stretch); }
};

int sample_half(PRNG& G);

template<class T>
class HalfGenerator : public RandomGenerator<T>
{
public:
  HalfGenerator(PRNG& G) :
    RandomGenerator<T>(G) {}
  Generator<T>* clone() const { return new HalfGenerator<T>(*this); }
  void get(T& x) const { x = sample_half(this->G); }
};
