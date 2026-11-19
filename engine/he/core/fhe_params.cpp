
#include "he/core/fhe_params.h"
#include "he/core/ntl_subs.h"
#include "he/core/ring_element.h"
#include "support/exceptions.h"
#include "protocols/semi/hemi_options.h"
#include "vm/online_options.h"

FheParameters::FheParameters(int n_mults, int drown_sec) :
    fft_data(n_mults + 1), Chi(0.7), sec_p(drown_sec), matrix_dimension(1)
{
}

void FheParameters::set(const Ring& R,
                     const vector<Bigint>& primes)
{
  if (primes.size() != fft_data.size())
    throw runtime_error("wrong number of primes");

  for (size_t i = 0; i < fft_data.size(); i++)
    fft_data[i].init(R,primes[i]);

  set_sec(sec_p);
}

void FheParameters::set_sec(int sec)
{
  assert(sec >= 0);
  sec_p=sec;
  bval=1;  bval=bval<<sec_p;
  bval=fft_data[0].get_prime()/(2*(1+bval));
}

void FheParameters::set_min_sec(int sec)
{
  set_sec(max(sec, sec_p));
}

void FheParameters::set_matrix_dimension(int matrix_dimension)
{
  assert(matrix_dimension > 0);
  if (fft_data[0].get_prime() != 0)
    throw runtime_error("cannot change matrix dimension after parameter generation");
  this->matrix_dimension = matrix_dimension;
}

void FheParameters::set_matrix_dimension_from_options()
{
  set_matrix_dimension(
      HemiOptions::singleton.plain_matmul ?
          1 : OnlineOptions::singleton.batch_size);
}

Bigint FheParameters::Q() const
{
  Bigint res = fft_data[0].get_prime();
  for (size_t i = 1; i < fft_data.size(); i++)
    res *= fft_data[i].get_prime();
  return res;
}

void FheParameters::pack(OctetStream& o) const
{
  o.store(fft_data);
  Chi.pack(o);
  bval.pack(o);
  o.store(sec_p);
  o.store(matrix_dimension);
  fd.pack(o);
}

void FheParameters::unpack(OctetStream& o)
{
  o.get(fft_data);
  Chi.unpack(o);
  bval.unpack(o);
  o.get(sec_p);
  o.get(matrix_dimension);
  fd.unpack(o);
}

bool FheParameters::operator!=(const FheParameters& other) const
{
  if (fft_data != other.fft_data or Chi != other.Chi or sec_p != other.sec_p
      or bval != other.bval)
    {
      return true;
    }
  else
    return false;
}

void FheParameters::basic_generation_mod_prime(int plaintext_length)
{
  if (n_mults() == 0)
    generate_semi_setup(plaintext_length, 0, *this, fd, false);
  else
    {
      Parameters parameters(1, plaintext_length, 0);
      parameters.generate_setup(*this, fd);
    }
}

template<>
const FftData& FheParameters::get_plaintext_field_data() const
{
  return fd;
}

template<>
const P2Data& FheParameters::get_plaintext_field_data() const
{
  throw NotImplemented();
}

Bigint FheParameters::get_plaintext_modulus() const
{
  return fd.get_prime();
}
