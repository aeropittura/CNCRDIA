#pragma once

/* This class defines all the data needed for representing
 * plaintexts modulo 2
 */

#include "he/core/ring.h"
#include "he/core/matrix.h"
#include "numeric/gf2n.h"

class P2Data
{
  int slots;
  // Data for the forward mapping  ((slots*Gf2n:deg) by phi_m)
  Imatrix A;
  // Data for the backward mapping (phi_m by phi_m)
  Imatrix Ai;

  public:
  typedef Gf2nShort T;
  typedef int S;
  typedef Fixint<0> poly_type;

  int num_slots() const { return slots; }
  int degree() const { return A.size() ? A.size() : 0; }
  int phi_m() const { return A[0].size(); }

  void check_dimensions() const;

  void forward(vector<poly_type>& ans,const vector<Gf2nShort>& a) const;
  void backward(vector<Gf2nShort>& ans,const vector<poly_type>& a) const;

  int get_prime() const { return 2; }
  void write_setup(const string&) const {}

  bool operator!=(const P2Data& other) const;

  // no op because we require field to be initalized first
  void initialize_field() const {}

  void hash(OctetStream& o) const;
  void pack(OctetStream& o) const;
  void unpack(OctetStream& o);

  void load(const Ring& rg);
  void store(const Ring& rg) const;

  void load_or_generate(const Ring& rg);


  friend void init(P2Data& P2D,const Ring& rg);
};
