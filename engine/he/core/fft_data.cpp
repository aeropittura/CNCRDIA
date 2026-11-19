#include "he/core/fft_data.h"
#include "he/core/fft.h"

#include "he/core/subroutines.h"

#include "numeric/modp.hpp"



FftData::FftData() :
    twop(-1)
{
}

void FftData::init(const Ring& rg,const ZpData& pr_d)
{
  R=rg;
  pr_data=pr_d;
  root.resize(2);

  // Find which case we are in 
  int hwt=Hwt(rg.m());
  Bigint prm1=pr_d.pr-1;
  twop=1<<(num_bits(rg.m())+1);
  if (hwt==1)
    { twop=0; }
  else if ((prm1%twop)!=0)
    { twop=-twop; }

  if (twop==0)
    { int nb=num_bits(rg.m());
      nb=1<<(nb-1);
      if (nb==rg.m())
        { //cout << Rg.m() << " " << pr_d.pr << endl;
          root[0]=find_primitive_root_2power(rg.m(),pr_d);
          Inv(root[1],root[0],pr_d);
          to_modp(iphi,rg.phi_m(),pr_d);
          Inv(iphi,iphi,pr_d);
          compute_roots(rg.m());
        }
    }
  else 
    { Bigint pr=pr_d.pr;
      if ((pr-1)%(2*rg.m())!=0)
	{ throw InvalidParameters(); }
      root[0]=find_primitive_root_2m(rg.m(),rg.Phi(),pr_d);
      Inv(root[1],root[0],pr_d); 
      compute_roots(2 * rg.m());

      int ptwop=twop; if (twop<0) { ptwop=-twop; }

      powers.resize(2,vector<modp>(rg.m()));
      powers_i.resize(2,vector<modp>(rg.m()));
      b.resize(2,vector<modp>(ptwop));

      modp r_inverse,bi;
      Bigint ee=ptwop; ee=ee*rg.m();
      for (int r=0; r<2; r++)
        { assignOne(powers[r][0],pr_d);
          if (r==0)
            { to_modp(powers_i[0][0],ptwop,pr_d);  }
          else
            { to_modp(powers_i[1][0],ee,pr_d);  }
          Inv(powers_i[r][0],powers_i[r][0],pr_d);

          Inv(r_inverse,root[r],pr_d);
          assignOne(b[r][rg.m()-1],pr_d);
          for (long i=1; i<rg.m(); i++)
            { long iSqr=(i*i)%(2*rg.m());
              Power(powers[r][i],root[r],iSqr,pr_d);
              Mul(powers_i[r][i],powers[r][i],powers_i[r][0],pr_d);
              Power(bi,r_inverse,iSqr,pr_d);
              b[r][rg.m()-1+i]=bi;
              b[r][rg.m()-1-i]=bi;
            }
       }

      if (twop>0)
        { two_root.resize(2);
          two_root[0]=find_primitive_root_2power(twop,pr_d);
          Inv(two_root[1],two_root[0],pr_d);

          for (int r=0; r<2; r++)
            { fft_iter(b[r],twop,two_root[0],pr_d);  }
        }
      else
        throw BadValue();
    }
}

void FftData::compute_roots(int n)
{
  roots.resize(n + 1);
  assignOne(roots[0], pr_data);
  for (int i = 1; i < n + 1; i++)
    Mul(roots[i], roots[i - 1], root[0], pr_data);
}


void FftData::hash(OctetStream& o) const
{
  OctetStream tmp;
  pack(tmp);
  o.concat(tmp.hash());
}


void FftData::pack(OctetStream& o) const
{
  R.pack(o);
  pr_data.pack(o);
  o.store(root);
  o.store(roots);
  o.store(twop);
  o.store(two_root);
  o.store(b);
  iphi.pack(o);
  o.store(powers);
  o.store(powers_i);
}


void FftData::unpack(OctetStream& o)
{
  R.unpack(o);
  pr_data.unpack(o);
  o.get(root);
  o.get(roots);
  o.get(twop);
  o.get(two_root);
  o.get(b);
  iphi.unpack(o);
  o.get(powers);
  o.get(powers_i);
}

bool FftData::operator!=(const FftData& other) const
{
  if (R != other.R or pr_data != other.pr_data or root != other.root
      or twop != other.twop or two_root != other.two_root or b != other.b
      or iphi != other.iphi or powers != other.powers or powers_i != other.powers_i)
    {
      return true;
    }
  else
    return false;
}
