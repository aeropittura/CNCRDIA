#pragma once

#include "he/offline/proof.h"

template <class FD>
bool check_decoding(const vector<Proof::BoundType>& AE, bool Diag, FD& field_d);

/* Defines the Verifier */
template <class FD>
class Verifier
{
  AddableVector<typename Proof::BoundType> z;
  AddableMatrix<IntRandomCoins::value_type::value_type> t;

  Proof& P;
  const FD& field_d;

public:
  Verifier(Proof& proof, const FD& field_d);

  void stage_2(
      AddableVector<Ciphertext>& c, OctetStream& ciphertexts,
      OctetStream& cleartexts,const FHE_PK& pk);

  /* This is the non-interactive version using the ROM
      - Creates space for all output values
      - Diag flag mirrors that in Prover
  */
  void nizk_pok(AddableVector<Ciphertext>& c,OctetStream& ciphertexts,OctetStream& cleartexts,
               const FHE_PK& pk, int prover);

  size_t report_size(ReportType type) { return z.report_size(type) + t.report_size(type); }
};
