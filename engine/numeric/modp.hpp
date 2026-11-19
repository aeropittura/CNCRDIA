#pragma once

#include "numeric/zp_data.h"
#include "numeric/modp.h"
#include "numeric/z2k.hpp"
#include "numeric/gfpvar.h"

#include "support/exceptions.h"

/***********************************************************************
 *  The following functions remain the same in Real and Montgomery rep *
 ***********************************************************************/

template<int L>
Modp_<L> Modp_<L>::add(const Modp_& other, const ZpData& zp_d) const
{
  Modp_ res;
  Add(res, *this, other, zp_d);
  return res;
}

template<int L>
Modp_<L> Modp_<L>::sub(const Modp_& other, const ZpData& zp_d) const
{
  Modp_ res;
  Sub(res, *this, other, zp_d);
  return res;
}

template<int L>
Modp_<L> Modp_<L>::mul(const Modp_& other, const ZpData& zp_d) const
{
  Modp_ res;
  Mul(res, *this, other, zp_d);
  return res;
}

template<int L>
void Modp_<L>::randomize(PRNG& G, const ZpData& zp_d)
{
  const int M = sizeof(mp_limb_t) * L;
  switch (zp_d.pr_byte_length)
  {
#define X(LL) case LL: G.random_bnd<LL>(x, zp_d.get_pr_a(), zp_d.overhang_mask()); break;
  X(M) X(M-1) X(M-2) X(M-3) X(M-4) X(M-5) X(M-6) X(M-7)
#undef X
  default:
    G.random_bnd(x, zp_d.get_pr_a(), zp_d.pr_byte_length, zp_d.overhang_mask());
  }
}

template<int L>
void Modp_<L>::unpack(OctetStream& o,const ZpData& zp_d)
{
  o.consume((Octet*) x,zp_d.t*sizeof(mp_limb_t));
}

template<int L>
template<int LL>
void Modp_<L>::unpack(OctetStream& o)
{
  o.consume((Octet*) x, LL);
}

template<int L>
template<int LL>
void Modp_<L>::pack(OctetStream& o) const
{
  o.append((Octet*) x, LL);
}

template<int L>
void Negate(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d)
{ 
  if (isZero(x,zp_d)) { ans=x; return; }
  mpn_sub_n(ans.x,zp_d.pr_a,x.x,zp_d.t); 
}


template<int L>
bool areEqual(const Modp_<L>& x,const Modp_<L>& y,const ZpData& zp_d)
{ if (mpn_cmp(x.x,y.x,zp_d.t)!=0)
    { return false; }
  return true;
}

template<int L>
bool isZero(const Modp_<L>& ans,const ZpData& zp_d)
{
  for (int i=0; i<zp_d.t; i++)
    { if (ans.x[i]!=0) { return false; } }
  return true;
}



/***********************************************************************
 *  All the remaining functions have Montgomery variants which we need *
 *  to deal with                                                       *
 ***********************************************************************/


template<int L>
void assignOne(Modp_<L>& x,const ZpData& zp_d)
{ if (zp_d.montgomery)
    { mpn_copyi(x.x,zp_d.R,zp_d.get_t()); }
  else
    { assignZero(x,zp_d); 
      x.x[0]=1; 
    }
}


template<int L>
bool isOne(const Modp_<L>& x,const ZpData& zp_d)
{ if (zp_d.montgomery)
    { if (mpn_cmp(x.x,zp_d.R,zp_d.t)!=0)
        { return false; }
    }
  else
    { if (x.x[0]!=1) { return false; }
      for (int i=1; i<min(zp_d.t, L); i++)
	{ if (x.x[i]!=0) { return false; } }
    }
  return true;
}



template<int L>
void Modp_<L>::to_bigint(Bigint& ans,const ZpData& zp_d,bool reduce) const
{ 
  auto& x = *this;
  mpz_ptr a = ans.get_mpz_t();
  if (a->_mp_alloc < zp_d.t)
      mpz_realloc(a, zp_d.t);
  if (zp_d.montgomery)
    {
      mp_limb_t one[MAX_MOD_SZ];
      inline_mpn_zero(one,L);
      one[0]=1;
      zp_d.mont_mult(a->_mp_d,x.x,one);
    }
  else
    { inline_mpn_copyi(a->_mp_d,x.x,zp_d.t); }
  a->_mp_size=zp_d.t;
  if (reduce)
    while (a->_mp_size>=1 && (a->_mp_d)[a->_mp_size-1]==0)
      { a->_mp_size--; }
}


template<int L>
template<int M>
void Modp_<L>::to_bigint(Bigint& ans,const ZpData& zp_d,bool reduce) const
{
  assert(M == zp_d.t);
  auto& x = *this;
  mpz_ptr a = ans.get_mpz_t();
  if (a->_mp_alloc < M)
      mpz_realloc(a, M);
  if (zp_d.montgomery)
    {
      mp_limb_t one[M];
      inline_mpn_zero(one,M);
      one[0]=1;
      zp_d.mont_mult_<M>(a->_mp_d,x.x,one);
    }
  else
    { inline_mpn_copyi(a->_mp_d,x.x,M); }
  a->_mp_size=M;
  if (reduce)
    while (a->_mp_size>=1 && (a->_mp_d)[a->_mp_size-1]==0)
      { a->_mp_size--; }
}


template<int L>
void to_modp(Modp_<L>& ans,int x,const ZpData& zp_d)
{
  inline_mpn_zero(ans.x,zp_d.get_t());
  if (x>=0)
    { ans.x[0]=x;
      if (zp_d.t==1) { ans.x[0]=ans.x[0]%zp_d.pr_a[0]; }
    }
  else
    { if (zp_d.t==1)
	{ ans.x[0]=(zp_d.pr_a[0]+x)%zp_d.pr_a[0]; }
      else
        { Bigint& xx = Bigint::tmp = zp_d.pr+x;
          to_modp(ans,xx,zp_d);
          return;
        }
    }
  if (zp_d.montgomery)
    { zp_d.mont_mult(ans.x,ans.x,zp_d.R2); }
}


template<int L>
void to_modp(Modp_<L>& ans,const mpz_class& x,const ZpData& zp_d)
{
  if (x == 0)
  {
    assignZero(ans, zp_d);
    return;
  }
  else if (x == 1)
  {
    assignOne(ans, zp_d);
    return;
  }
  Bigint::tmp = x;
  ans.convert_destroy(Bigint::tmp, zp_d);
}


template<int L>
void Modp_<L>::convert_destroy(Bigint& xx,
    const ZpData& zp_d)
{
  xx %= zp_d.pr;
  //mpz_mod(xx.get_mpz_t(),x.get_mpz_t(),zp_d.pr.get_mpz_t());
  convert(xx.get_mpz_t()->_mp_d, abs(xx.get_mpz_t()->_mp_size), zp_d, xx < 0);
}

template<int L>
template<int M>
void Modp_<L>::convert_destroy(const Fixint<M>& xx,
    const ZpData& zp_d)
{
  assert(xx.size_in_limbs() <= L);
  SignedZ2<64 * L> tmp = xx;
  if (xx.negative())
    tmp += zp_d.pr;
  convert(tmp.get(), zp_d.get_t(), zp_d, false);
}

template<int L>
void Modp_<L>::convert(const mp_limb_t* source, mp_size_t size, const ZpData& zp_d, bool negative)
{
  assert(size <= zp_d.get_t());
  if (negative)
    mpn_sub(x, zp_d.pr_a, zp_d.t, source, size);
  else
    {
      inline_mpn_zero(x + size, zp_d.t - size);
      inline_mpn_copyi(x, source, size);
    }
  if (zp_d.montgomery)
    zp_d.mont_mult(x, x, zp_d.R2);
}

template<int L>
void Modp_<L>::zero_overhang(const ZpData& zp_d)
{
  x[zp_d.get_t() - 1] &= zp_d.overhang_mask();
}



template<int L>
void Sqr(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d)
{ 
  if (zp_d.montgomery)
    { zp_d.mont_mult(ans.x,x.x,x.x); }
  else
    { //ans.x=(x.x*x.x)%zp_d.pr;
      mp_limb_t aa[2*L],q[2*L];
      mpn_sqr(aa,x.x,zp_d.t);
      mpn_tdiv_qr(q,ans.x,0,aa,2*zp_d.t,zp_d.pr_a,zp_d.t);     
    }
}


template<int L>
void Inv(Modp_<L>& ans,const Modp_<L>& x,const ZpData& zp_d)
{ 
  mp_limb_t g[L],xx[L],yy[L];
  mp_size_t sz;
  mpn_copyi(xx,x.x,zp_d.t);
  mpn_copyi(yy,zp_d.pr_a,zp_d.t);
  mpn_gcdext(g,ans.x,&sz,xx,zp_d.t,yy,zp_d.t);
  if (sz<0)
    {
      assert(-sz <= L);
      mpn_sub(ans.x,zp_d.pr_a,zp_d.t,ans.x,-sz);
      sz=-sz;
    }
  else
    { for (int i=sz; i<zp_d.t; i++) { ans.x[i]=0; } }
  if (zp_d.montgomery)
    { zp_d.mont_mult_max(ans.x,ans.x,zp_d.R3,L); }
}


template<int L>
void Power(Modp_<L>& ans,const Modp_<L>& x,int exp,const ZpData& zp_d)
{
  if (exp==1) { ans=x; return; }
  if (exp==0) { assignOne(ans,zp_d); return; }
  if (exp<0)  { throw NotImplemented(); }
  Modp_<L> t=x;
  assignOne(ans,zp_d);
  while (exp!=0)
    { if ((exp&1)==1) { Mul(ans,ans,t,zp_d); }
      exp>>=1;
      Sqr(t,t,zp_d);
    }
}


template<int L>
void Power(Modp_<L>& ans,const Modp_<L>& x,const Bigint& exp,const ZpData& zp_d)
{
  if (exp==1) { ans=x; return; }
  if (exp==0) { assignOne(ans,zp_d); return; }
  if (exp<0)  { throw NotImplemented();  }
  modp t=x;
  assignOne(ans,zp_d);
  Bigint& e = Bigint::tmp = exp;
  while (e!=0)
     { if ((e&1)==1) { Mul(ans,ans,t,zp_d); }
       e>>=1;
       Sqr(t,t,zp_d);
     }
}


template<int L>
void Modp_<L>::output(ostream& s, const ZpData& zp_d, bool human, bool signed_) const
{
  if (human)
    { Bigint te;
      to_bigint(te, zp_d);
      if (te <= zp_d.pr_half or not signed_)
          s << te;
      else
          s << (te - zp_d.pr);
    }
  else
    { s.write((char*) x,zp_d.t*sizeof(mp_limb_t)); }
}

template<int L>
void Modp_<L>::input(istream& s,const ZpData& zp_d,bool human)
{
  if (s.peek() == EOF)
    { if (s.tellg() == 0)
        { cout << "IO problem. Empty file?" << endl;
          throw file_error("modp input");
        }
      throw EndOfFile("modp");
    }

  if (human)
    { Bigint te;
      s >> te;
      to_modp(*this,te,zp_d);
    }
  else
    { s.read((char*) x,zp_d.t*sizeof(mp_limb_t)); }
}
