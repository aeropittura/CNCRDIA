#pragma once

#include "he/offline/proof.h"
#include "support/memory_usage.h"

/* Class for the prover */

template<class FD, class U>
class Prover
{
  /* Provers state */
  Proof::Randomness s;
  AddableVector< Plaintext_<FD> > y;

#ifdef LESS_ALLOC_MORE_MEM
  AddableVector<typename Proof::BoundType> z;
  AddableMatrix<IntRandomCoins::value_type::value_type> t;
#endif

public:
  size_t volatile_memory;

  Prover(Proof& proof, const FD& field_d);

  void stage_1(const Proof& P, OctetStream& ciphertexts, const AddableVector<Ciphertext>& c,
      const FHE_PK& pk);

  bool stage_2(Proof& P, OctetStream& cleartexts,
               const vector<U>& x,
               const Proof::Randomness& r,
               const FHE_PK& pk);

  /* Only has a non-interactive version using the ROM 
      - If Diag is true then the plaintexts x are assumed to be
        diagonal elements, i.e. x=(x_1,x_1,...,x_1)
  */
  size_t nizk_pok(Proof& P, OctetStream& ciphertexts, OctetStream& cleartexts,
	       const FHE_PK& pk,
               const AddableVector<Ciphertext>& c,
               const vector<U>& x,
               const Proof::Randomness& r,
               int prover);

  size_t report_size(ReportType type);
  void report_size(ReportType type, MemoryUsage& res);
};
