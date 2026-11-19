#pragma once

/* Class to hold the FHE Parameters 
 *
 * The idea is that there is a single global FheParameters structure
 * called params; which holds all the data
 *
 * Likewise initialising params sets up all the data for other
 * classes such the global ring etc
 *
 */
#include "he/core/fft_data.h"
#include "he/core/discrete_gauss.h"
#include "support/random.h"
#include "protocols/common/config.h"

/**
 * Cryptosystem parameters
 */
class FheParameters
{
  protected:
  // Pair of moduli in FFTData[0] and FFTData[1] for the levels
  vector<FftData> fft_data;

  // Random generator for Multivariate Gaussian Distribution etc
  mutable DiscreteGauss Chi;

  // Data for distributed decryption
  int sec_p;
  Bigint bval;
  int matrix_dimension;

  FftData fd;

  public:

  /**
   * Initialization.
   * @param n_mults number of ciphertext multiplications (0/1)
   * @param drown_sec parameter for function privacy (default 40)
   */
  FheParameters(int n_mults = 1, int drown_sec = DEFAULT_SECURITY);

  int n_mults() const { return fft_data.size() - 1; }

  void set(const Ring& R,const vector<Bigint>& primes);
  void set(const vector<Bigint>& primes);
  void set_sec(int sec);
  void set_min_sec(int sec);

  void set_matrix_dimension(int matrix_dimension);
  void set_matrix_dimension_from_options();
  int get_matrix_dimension() const         { return matrix_dimension; }

  const vector<FftData>& FFTD() const { return fft_data; }

  const Bigint& p0() const           { return fft_data[0].get_prime(); }
  const Bigint& p1() const           { return fft_data[1].get_prime(); }
  Bigint Q() const;

  int secp() const                   { return sec_p;        }
  const Bigint& B() const            { return bval;         }
  double get_R() const               { return Chi.get_R();  }
  void set_r(double R) const         { return Chi.set(R); }
  DiscreteGauss get_dg() const       { return Chi; }

  int phi_m() const                  { return fft_data[0].phi_m(); }
  const Ring& get_ring()             { return fft_data[0].get_R(); }

  /// Append to buffer
  void pack(OctetStream& o) const;

  /// Read from buffer
  void unpack(OctetStream& o);

  bool operator!=(const FheParameters& other) const;

  /**
   * Generate parameter for computation modulo a prime
   * @param plaintext_length bit length of prime
   */
  void basic_generation_mod_prime(int plaintext_length);

  template<class FD>
  const FD& get_plaintext_field_data() const;

  Bigint get_plaintext_modulus() const;
};
