#include "he/core/rq_element.h"
#include "he/core/fhe_keys.h"
#include "support/exceptions.h"
#include "support/code_locations.h"

#include "numeric/modp.hpp"

RqElement::RqElement(const FHE_PK& pk) :
        RqElement(pk.get_parameters().FFTD(), evaluation, evaluation)
{
}

RqElement::RqElement(const vector<FftData>& prd, RepType r0, RepType r1)
{
    if (prd.size() > 0)
        a.push_back({prd[0], r0});
    if (prd.size() > 1)
    {
        assert(prd[0].get_R() == prd[1].get_R());
        a.push_back({prd[1], r1});
    }
    lev = n_mults();
}

void RqElement::set_data(const vector<FftData>& prd)
{
    a.resize(prd.size(), {});
    for(size_t i = 0; i < a.size(); i++)
        a[i].set_data(prd[i]);
    lev=n_mults();
}

void RqElement::assign_zero(const vector<FftData>& prd)
{
    set_data(prd);
    assign_zero();
}

void RqElement::assign_zero()
{
	for (int i=0; i<=lev; ++i)
		a[i].assign_zero();
}

void RqElement::assign_one()
{
	for (int i=0; i<=lev; ++i)
		a[i].assign_one();
}

void RqElement::partial_assign(const RqElement& other)
{
	lev=other.lev;
	a.resize(other.a.size(), {});
}

void RqElement::negate()
{
	for (int i=0; i<=lev; ++i)
		a[i].negate();
}

RqElement RqElement::mult_by_x_i(int i) const
{
  RqElement res;
  res.lev = lev;
  res.a.clear();
  for (auto& x : a)
    {
      auto tmp = x.mult_by_x_i(i);
      res.a.push_back(tmp);
    }
  return res;
}

void add(RqElement& ans,const RqElement& ra,const RqElement& rb)
{
  ans.partial_assign(ra, rb);
  for (int i=0; i<=ans.lev; ++i)
	  add(ans.a[i],ra.a[i],rb.a[i]);
  if (ans.lev == 0 && ans.n_mults() == 1) {
	  ans.a[1].partial_assign(ra.a[1]);
  }
}
void sub(RqElement& ans,const RqElement& a,const RqElement& b)
{
  ans.partial_assign(a, b);
  for (int i=0; i<=ans.lev; ++i)
	  sub(ans.a[i],a.a[i],b.a[i]);
  if (ans.lev == 0 && ans.n_mults() == 1) {
	  ans.a[1].partial_assign(a.a[1]);
  }
}

void mul(RqElement& ans,const RqElement& a,const RqElement& b)
{
  ans.partial_assign(a, b);
  for (int i=0; i<=ans.lev; ++i)
	  mul(ans.a[i],a.a[i],b.a[i]);
  if (ans.lev == 0 && ans.n_mults() == 1) {
	  ans.a[1].partial_assign(a.a[1]);
  }
}

void mul(RqElement& ans,const RqElement& a,const Bigint& b)
{
  ans.partial_assign(a);
  modp bp;
  for (int i=0; i<=ans.lev; ++i)
  {
	  to_modp(bp,b,a.a[i].get_pr_d());
	  mul(ans.a[i],a.a[i],bp);
  }
}

void RqElement::randomize(PRNG& G,int l)
{
  set_level(l);
  for (int i=0; i<=lev; ++i)
	  a[i].randomize(G);
}

bool RqElement::equals(const RqElement& other) const
{
  if (lev!=other.lev)                 { throw LevelMismatch(); }
  for (int i=0; i<=lev; ++i)
	  if (!a[i].equals(other.a[i])) return false;
  return true;
}


vector<Bigint> RqElement::to_vector_bigint() const
{
  vector<Bigint> v;
  to_vector_bigint(v);
  return v;
}

// Doing sort of CRT;
// result mod p0 = a[0]; result mod p1 = a[1]
void RqElement::to_vector_bigint(vector<Bigint>& v) const
{
  CODE_LOCATION_NO_SCOPE

  a[0].to_vector_bigint(v);
  if (n_mults() == 0) {
	  Bigint p0 = a[0].get_prime();
	  for (size_t i = 0; i < v.size(); ++i) {
		  if (v[i] > p0 / 2) {
			  v[i] = (v[i] - p0);
		  }
	  }
  }
  if (lev==1)
    { vector<Bigint> v1;
      a[1].to_vector_bigint(v1);
      assert(v.size() == v1.size());
      Bigint p0=a[0].get_prime();
      Bigint p1=a[1].get_prime();
      Bigint p0i,lambda,Q=p0*p1;
      inverse_mod(p0i,p0%p1,p1);
      for (unsigned int i=0; i<v.size(); i++)
	{ lambda=((v1[i]-v[i])*p0i)%Q;
          v[i]=(v[i]+p0*lambda)%Q;
	}
    }
}

ConversionIterator RqElement::get_iterator() const
{
  if (lev != 0)
    throw NotImplemented();
  return a[0].get_iterator();
}

Bigint RqElement::infinity_norm() const
{
  Bigint Q = 1, ans = 0;
  for (int i = 0; i <= n_mults(); ++i)
  {
	  Q *= a[i].get_prime();
  }
  Bigint t;
  vector<Bigint> te=to_vector_bigint();
  for (unsigned int i=0; i<te.size(); i++)
    { // Take rounded value and then abs value
      if (te[i]<Q/2) { t=te[i]; }
      else           { t=Q-te[i]; }
      if (t>ans) { ans=t; }
    }
  return ans;
}

void RqElement::change_rep(RepType r)
{
  if (lev==1) { throw LevelMismatch(); }
  a[0].change_rep(r);
}

void RqElement::change_rep(RepType r0,RepType r1)
{
  if (lev==0 or n_mults() != 1) { throw LevelMismatch(); }
  a[0].change_rep(r0);
  a[1].change_rep(r1);
}

void RqElement::Scale(const Bigint& p)
{
  if (lev==0) { return; }

  if (a[0].is_zero() and a[1].is_zero())
  {
      lev = 0;
      return;
  }

  CODE_LOCATION_NO_SCOPE

  if (n_mults() == 0) {
	  //for some reason we scale but we have just one level
	  throw LevelMismatch();
  }
  Bigint p0=a[0].get_prime(),p1=a[1].get_prime(),p1i,lambda,n=p1*p;
  inverse_mod(p1i,p1%p,p);

  // First multiply input by [p1]_p
  Bigint te=p1%p;
  if (te>p/2) { te-=p; }
  modp tep;
  to_modp(tep,te,a[0].get_pr_d());
  mul(a[0],a[0],tep);
  to_modp(tep,te,a[1].get_pr_d());
  mul(a[1],a[1],tep);

  // Now compute delta
  RingElement b0(a[0].get_fftd(),evaluation);
  RingElement b1(a[1].get_fftd(),evaluation);
  // scope to ensure deconstruction of write iterators
  {
    auto polynomial_a1 = a[1];
    polynomial_a1.change_rep(polynomial);
    auto it = polynomial_a1.get_iterator();
    auto it0 = b0.get_write_iterator();
    auto it1 = b1.get_write_iterator();
    Bigint half_n = n / 2;
    Bigint delta;
    for (int i=0; i < a[1].get_fftd().phi_m(); i++)
      {
        it.get(delta);
        lambda = delta;
        lambda *= p1i;
        lambda %= p;
        lambda *= p1;
        lambda -= delta;
        lambda %= n;
        if (lambda > half_n)
          lambda -= n;
        it0.get(lambda);
        it1.get(lambda);
      }
  }

  // Now add delta back onto a0
  RqElement bb(b0,b1);
  ::add(*this,*this,bb);

  // Now divide by p1 mod p0
  modp p1_inverse,pp;
  to_modp(pp,p1,a[0].get_pr_d());
  Inv(p1_inverse,pp,a[0].get_pr_d());
  lev=0;
  mul(a[0],a[0],p1_inverse);
}

void RqElement::mult_by_p1()
{

  if (n_mults() == 0) {throw LevelMismatch();}
  lev=1;
  Bigint m=a[1].get_prime()%a[0].get_prime();
  modp mp;
  to_modp(mp,m,a[0].get_pr_d());
  mul(a[0],a[0],mp);
  a[1].assign_zero();
}

void RqElement::raise_level()
{
  if (lev==n_mults()) { return; }
  lev=1;
  a[1].from(a[0].get_copy_iterator());
}

void RqElement::check_level() const
{
  if ((unsigned)lev > (unsigned)n_mults())
    throw range_error(
        "level out of range: " + to_string(lev) + "/" + to_string(n_mults()));
}

void RqElement::partial_assign(const RqElement& x, const RqElement& y)
{
  x.check_level();
  y.check_level();
  if (x.lev != y.lev or x.n_mults() != y.n_mults())
    throw LevelMismatch();
  partial_assign(x);
}

void RqElement::pack(OctetStream& o, int) const
{
  check_level();
  o.store(lev);
  for (int i = 0; i <= lev; ++i)
	  a[i].pack(o);
}

void RqElement::unpack(OctetStream& o, int)
{
  unsigned int ll;  o.get(ll); lev=ll;

  try
  {
      check_level();
  }
  catch (...)
  {
      lev = 0;
      throw;
  }

  for (int i = 0; i <= lev; ++i)
	  a[i].unpack(o);
}

void RqElement::output(ostream& s) const
{
  check_level();
  s.write((char*)&lev, sizeof(lev));
  for (int i = 0; i <= lev; i++)
    a[i].output(s);
}

void RqElement::input(istream& s)
{
  s.read((char*)&lev, sizeof(lev));
  check_level();
  for (int i = 0; i <= lev; i++)
    a[i].input(s);
}

void RqElement::check(const FheParameters& params) const
{
  if (n_mults() != params.n_mults())
    throw LevelMismatch();
  for (int i = 0; i <= lev; i++)
    a[i].check(params.FFTD()[i]);
}

size_t RqElement::report_size(ReportType type) const
{
  size_t sz = a[0].report_size(type);
  if (lev == 1 || type == CAPACITY)
	  if (n_mults() == 1)
		  sz += a[1].report_size(type);
  return sz;
}

void RqElement::unpack(OctetStream& o, const FheParameters& params)
{
  set_data(params.FFTD());
  unpack(o);
}

void RqElement::print_first_non_zero() const
{
  vector<Bigint> v = to_vector_bigint();
  size_t i;
  for (i = 0; i < v.size(); i++)
  {
      if (v[i] != 0)
      {
          cout << i << ":" << v[i];
          break;
      }
  }
  if (i == v.size())
      cout << "ZERO" << endl;
  cout << endl;
}

template void RqElement::from<Bigint>(const Generator<Bigint>&, int);
template void RqElement::from<int>(const Generator<int>&, int);
