
#pragma once

#include <vector>
using namespace std;

#include "numeric/bigint.h"
#include "numeric/modp.h"
#include "support/bit_vector.h"

typedef vector< vector<Bigint> > matrix;

class Imatrix : public vector< BitVector >
{
    typedef vector<BitVector> Super;

public:
    bool operator!=(const Imatrix& other) const;

    void hash(OctetStream& o) const;
    void pack(OctetStream& o) const;
    void unpack(OctetStream& o);
};

/* Uses Algorithm 2.7 from Pohst-Zassenhaus to compute H and U st
		H = HNF(A) = A*U
*/
void HNF(matrix& H,matrix& U,const matrix& A);

/* S = U*A*V
   We dont care about U though
*/
void SNF(matrix& S,const matrix& A,matrix& V);

void print(const matrix& S);
void print(const Imatrix& S);

// Special inverse routine, assumes A is unimodular and only
// requires column operations to create the inverse
matrix inv(const matrix& A);

// Finds a pseudo-inverse of a matrix A modulo 2
//   - Input matrix is assumed to have more rows than columns
void pinv(Imatrix& Ai,const Imatrix& A);
