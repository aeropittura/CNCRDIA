#pragma once


#include "numeric/zp_data.h"
#include "numeric/modp.h"

void Subs(modp& ans,const vector<int>& poly,const modp& x,const ZpData& zp_d);


/* Find an m'th primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 *   poly is Phi_m(X)
 */
modp find_primitive_root_m(int m,const vector<int>& poly,const ZpData& zp_d);


/* Find a (2m)'th primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 *   poly is Phi_m(X)
 */
modp find_primitive_root_2m(int m,const vector<int>& poly,const ZpData& zp_d);


/* Find an mth primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 * This assumes m is a power of two and so the cyclotomic polynomial
 * is  F=X^{m/2}+1
 */
modp find_primitive_root_2power(int m,const ZpData& zp_d);
