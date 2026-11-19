
#include "he/core/subroutines.h"
#include "numeric/modp.h"

#include "numeric/modp.hpp"

void Subs(modp& ans,const vector<int>& poly,const modp& x,const ZpData& zp_d)
{
  modp one,co;
  assignOne(one,zp_d);
  assignZero(ans,zp_d);
  for (int i=poly.size()-1; i>=0; i--)
    { Mul(ans,ans,x,zp_d);
      if (poly[i] > 0)
        {
          for (int j = 0; j < poly[i]; j++)
            Add(ans, ans, one, zp_d);
        }
      if (poly[i] < 0)
        {
          for (int j = 0; j < -poly[i]; j++)
            Sub(ans, ans, one, zp_d);
        }
    }
}



/* Find a m'th primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 *   poly is Phi_m(X)
 */
modp find_primitive_root_m(int m,const vector<int>& poly,const ZpData& zp_d)
{
  modp ans,e,one,base;
  assignOne(one,zp_d);
  assignOne(base,zp_d);
  Bigint   exp;
  exp=(zp_d.pr-1)/m;
  bool flag=true;
  while (flag)
    { Add(base,base,one,zp_d);   // Keep incrementing base until we hit the answer
      Power(ans,base,exp,zp_d);
      // e=Phi(ans)
      Subs(e,poly,ans,zp_d);
      if (isZero(e,zp_d)) { flag=false; }
    }
  return ans;
}



/* Find a (2m)'th primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 *   poly is Phi_m(X)
 */
modp find_primitive_root_2m(int m,const vector<int>& poly,const ZpData& zp_d)
{
  // Thin out poly, where poly is Phi_m(X) by 2
  vector<int> poly2;
  poly2.resize(2*poly.size());
  for (unsigned int i=0; i<poly.size(); i++)
    { poly2[2*i]=poly[i];
      poly2[2*i+1]=0;
    }
  modp ans=find_primitive_root_m(2*m,poly2,zp_d);
  return ans;
}



/* Find an mth primitive root moduli the current prime
 *   This is deterministic so all players have the same root of unity
 * This assumes m is a power of two and so the cyclotomic polynomial
 * is  F=X^{m/2}+1
 */
modp find_primitive_root_2power(int m,const ZpData& zp_d)
{
  assert((m & (m - 1)) == 0);
  assert(m > 1);
  modp ans,e,one,base;
  assignOne(one,zp_d);
  assignOne(base,zp_d);
  Bigint   exp;
  exp=(zp_d.pr-1)/m;
  assert(exp * m == zp_d.pr - 1);
  bool flag=true;
  while (flag)
    { Add(base,base,one,zp_d);   // Keep incrementing base until we hit the answer
      Power(ans,base,exp,zp_d);
      // e=ans^{m/2}+1
      Power(e,ans,m/2,zp_d);
      Add(e,e,one,zp_d);
      if (isZero(e,zp_d)) { flag=false; }
    }
  return ans;
}
