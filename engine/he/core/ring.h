#pragma once

/* Class to define the basic ring we are working with
 * and any associated data
 */

#include <vector>
#include <iostream>
#include <assert.h>
using namespace std;

#include "support/octet_stream.h"

class Ring
{
  int mm,phim;
  vector<int> pi;
  vector<int> pi_inverse;
  vector<int> poly;
  
  public:


  Ring(int m = 0);
  ~Ring()     { ; }

  // Rely on default copy assignment/constructor
  
  int phi_m() const { return phim; }
  int m()     const { return mm;   }

  int p(int i)      const { return pi.at(i);     }
  int p_inverse(int i)  const { return pi_inverse.at(i); }
  const vector<int>& Phi() const { assert(poly.size()); return poly; }

  friend void init(Ring& rg, int m, bool generate_polynomial);

  void pack(OctetStream& o) const;
  void unpack(OctetStream& o);

  bool operator!=(const Ring& other) const;
  bool operator==(const Ring& other) const { return not (*this != other); }
};

void init(Ring& rg, int m, bool generate_polynomial = false);
