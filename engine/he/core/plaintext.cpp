
#include "he/core/plaintext.h"
#include "he/core/ring_element.h"
#include "he/core/p2_data.h"
#include "he/core/rq_element.h"
#include "he/core/fhe_keys.h"
#include "he/core/addable_vector.hpp"
#include "numeric/z2k.hpp"
#include "numeric/modp.hpp"



template<class T, class FD, class S>
Plaintext<T, FD, S>::Plaintext(const FheParameters& params) :
    Plaintext(params.get_plaintext_field_data<FD>(), Both)
{
}


template<class T, class FD, class S>
unsigned int Plaintext<T, FD, S>::num_slots() const
{
  return (*field_data).phi_m();
}

template<class T, class FD, class S>
int Plaintext<T, FD, S>::degree() const
{
  return (*field_data).phi_m();
}


template<>
unsigned int Plaintext<Gf2nShort,P2Data,int>::num_slots() const
{
  return (*field_data).num_slots();
}

template<>
int Plaintext<Gf2nShort,P2Data,int>::degree() const
{
  return (*field_data).degree();
}


template<>
void Plaintext<Gfp, FftData, Bigint>::from(const Generator<Bigint>& source) const
{
  b.resize(degree());
  for (auto& x : b)
    {
      source.get(Bigint::tmp);
      x = Bigint::tmp;
    }
}


template<>
void Plaintext<Gfp,FftData,Bigint>::from_polynomial() const
{
  if (type!=Polynomial) { return; }

  RingElement e(*field_data,polynomial);
  e.from(b);
  e.change_rep(evaluation);
  a.resize(num_slots());
  for (unsigned int i=0; i<a.size(); i++)
    a[i] = Gfp(e.get_element(i), e.get_fftd().get_pr_d());
  type=Both;
}


template<>
void Plaintext<Gfp,FftData,Bigint>::to_polynomial() const
{
  if (type!=Evaluation) { return; }

  RingElement e(*field_data,evaluation);
  for (unsigned int i=0; i<a.size(); i++)
    { e.set_element(i,a[i].get()); }
  e.change_rep(polynomial);
  from(e.get_iterator());
  type=Both;
}



template<>
void Plaintext<Gf2nShort,P2Data,int>::from_polynomial() const
{ 
  if (type!=Polynomial) { return; }
  a.resize(num_slots());
  (*field_data).backward(a,b); 
  type=Both;
}


template<>
void Plaintext<Gf2nShort,P2Data,int>::to_polynomial() const
{ 
  if (type!=Evaluation) { return; }
  (*field_data).forward(b,a); 
  type=Both;
}



template<class T, class FD, class S>
void Plaintext<T, FD, S>::allocate(PtType type) const
{
  if (type != Evaluation)
    b.resize(degree());
  if (type != Polynomial)
    a.resize(num_slots());
  this->type = type;
}


template<class T, class FD, class S>
void Plaintext<T, FD, S>::allocate_slots(const Bigint& value)
{
  b.resize(degree());
  for (auto& x : b)
    x.allocate_slots(value);
}

template<>
void Plaintext<Gf2nShort, P2Data, int>::allocate_slots(const Bigint& value)
{
  // nothing to allocate for int
  (void)value;
}

template<>
int Plaintext<Gfp, FftData, Bigint>::get_min_alloc()
{
  int res = 1 << 30;
  for (auto& x : b)
    res = min(res, x.get_min_alloc());
  return res;
}


void signed_mod(Bigint& x, const Bigint& mod, const Bigint& half_mod, const Bigint& destination_mod)
{
  if (x > half_mod)
    x -= mod;
  x %= destination_mod;
  if (x < 0)
    x += destination_mod;
}

template<class T, class FD, class S>
void Plaintext<T, FD, S>::set_polynomial_mod(const Generator<Bigint>& generator,const Bigint& mod)
{
  allocate(Polynomial);
  Bigint half_mod = mod / 2;
  for (unsigned int i=0; i<b.size(); i++)
    {
      generator.get(Bigint::tmp);
      signed_mod(Bigint::tmp, mod, half_mod, field_data->get_prime());
      b[i] = Bigint::tmp;
    }
}




template<>
void Plaintext<Gf2nShort,P2Data,int>::set_polynomial_mod(const vector<Bigint>& vv,const Bigint& mod)
{
  vector<P2Data::poly_type> pol(vv.size());
  Bigint te;
  for (unsigned int i=0; i<vv.size(); i++)
    { if (vv[i]>mod/2) { te=vv[i]-mod; }
      else             { te=vv[i];     }
      pol[i]=is_odd(te);
    }
  set_polynomial(pol);
}


template<>
void Plaintext<Gf2nShort,P2Data,int>::set_polynomial_mod(const Generator<Bigint>& generator,const Bigint& mod)
{
  allocate(Polynomial);
  Bigint half_mod = mod / 2;
  Bigint te;
  for (unsigned int i=0; i<b.size(); i++)
    {
      generator.get(te);
      if (te > half_mod)
        te -= mod;
      b[i]=is_odd(te);
    }
}


template<class T>
void random_polynomial(vector<T>& b,PRNG& G,const Bigint& pr,bool positive=true)
{
  for (unsigned int i=0; i<b.size(); i++)
    {
      b[i].random_bnd(G, pr, positive);
    }
}

template<class T,class FD,class S>
void Plaintext<T,FD,S>::randomize(PRNG& G,condition cond)
{
  switch (cond)
    { case Full:
        random_polynomial(b,G,(*field_data).get_prime());
        type=Polynomial;
        break;
      case Diagonal:
        a.resize(num_slots());
        a[0].randomize(G);
        for (unsigned int i=1; i<a.size(); i++)
           { a[i]=a[0]; }
        type=Evaluation;
        break;
      default:
        // Gen a plaintext with 0/1 in each slot
        a.resize(num_slots());
	for (unsigned int i=0; i<a.size(); i++)
           {
             if (G.get_bit())
		{ a[i].assign_one(); }
             else
	        { a[i].assign_zero(); }
           }
        type=Evaluation;
        break;
    }
    
}


template<class T,class FD,class S>
void Plaintext<T,FD,S>::randomize(PRNG& G, int n_bits, bool Diag, PtType t)
{
  allocate(t);
  switch(t)
  {
    case Polynomial:
      if (Diag)
        {
          assign_zero(t);
          b[0].generate_uniform(G, n_bits, false);
        }
      else
        for (size_t i = 0; i < num_slots(); i++)
          b[i].generate_uniform(G, n_bits, false);
      break;
    default:
      throw NotImplemented();
  }
}


template<class T,class FD,class S>
void Plaintext<T,FD,S>::assign_zero(PtType t)
{ 
  type=t;
  allocate();
  if (type!=Polynomial)
    {
      a.resize(num_slots());
      for (unsigned int i=0; i<a.size(); i++)
        { a[i].assign_zero(); }
    }
  if (type!=Evaluation)
    { for (unsigned int i=0; i<b.size(); i++)
        { b[i]=0; }
    }

}

template<class T,class FD,class S>
void Plaintext<T,FD,S>::assign_one(PtType t)
{ 
  type=t;
  allocate();
  if (type!=Polynomial)
    {
      a.resize(num_slots());
      for (unsigned int i=0; i<a.size(); i++)
        { a[i].assign_one(); }
    }
  if (type!=Evaluation)
    { for (unsigned int i=1; i<b.size(); i++)
        { b[i]=0; }
      b[0]=1;
    }
}

template<class T,class FD,class S>
void Plaintext<T,FD,S>::assign_constant(T constant, PtType t)
{
  allocate(Evaluation);
  for (auto& x : a)
      x = constant;
  if (t != Evaluation)
      to_polynomial();
}

template<class T,class FD,class S>
Plaintext<T,FD,S>& Plaintext<T,FD,S>::operator+=(
    const Plaintext& y)
{
  if (field_data!=y.field_data)  { throw FieldMismatch(); }

  to_polynomial();
  y.to_polynomial();

  if (b.size() != y.b.size())
    throw length_error("size mismatch");

  add(*this, *this, y);
  return *this;
}


template<>
void add(Plaintext<Gfp,FftData,Bigint>& z,const Plaintext<Gfp,FftData,Bigint>& x,
                                           const Plaintext<Gfp,FftData,Bigint>& y)
{
  if (z.field_data!=x.field_data)  { throw FieldMismatch(); }
  if (z.field_data!=y.field_data)  { throw FieldMismatch(); }

  if (x.type==Both && y.type!=Both)      { z.type=y.type; }
  else if (y.type==Both && x.type!=Both) { z.type=x.type; }
  else if (x.type!=y.type)               { throw RepMismatch(); }
  else                                   { z.type=x.type; }

  z.allocate();
  if (z.type!=Polynomial)
    {
      z.a.resize(z.num_slots());
      for (unsigned int i=0; i<z.a.size(); i++)
        { z.a[i] = (x.a[i] + y.a[i]); }
    }
  if (z.type!=Evaluation)
    { for (unsigned int i=0; i<z.b.size(); i++)
        { z.b[i]=x.b[i]+y.b[i];
          if (z.b[i]>(*z.field_data).get_prime())
	    { z.b[i]-=(*z.field_data).get_prime(); }
	}
    }
}




template<>
void add(Plaintext<Gf2nShort,P2Data,int>& z,const Plaintext<Gf2nShort,P2Data,int>& x,
                                       const Plaintext<Gf2nShort,P2Data,int>& y)
{
  if (z.field_data!=x.field_data)  { throw FieldMismatch(); }
  if (z.field_data!=y.field_data)  { throw FieldMismatch(); }

  if (x.type==Both && y.type!=Both)      { z.type=y.type; }
  else if (y.type==Both && x.type!=Both) { z.type=x.type; }
  else if (x.type!=y.type)               { throw RepMismatch(); }
  else                                   { z.type=x.type; }

  z.allocate();
  if (z.type!=Polynomial)
    {
      z.a.resize(z.num_slots());
      for (unsigned int i=0; i<z.a.size(); i++)
        { z.a[i].add(x.a[i],y.a[i]); }
    }
  if (z.type!=Evaluation)
    { for (unsigned int i=0; i<z.b.size(); i++)
        {
          z.b[i]=x.b[i] ^ y.b[i];
        }
    }
}


template<>
void sub(Plaintext<Gfp,FftData,Bigint>& z,const Plaintext<Gfp,FftData,Bigint>& x,
                                           const Plaintext<Gfp,FftData,Bigint>& y)
{
  if (z.field_data!=x.field_data)  { throw FieldMismatch(); }
  if (z.field_data!=y.field_data)  { throw FieldMismatch(); }

  if (x.type==Both && y.type!=Both)      { z.type=y.type; }
  else if (y.type==Both && x.type!=Both) { z.type=x.type; }
  else if (x.type!=y.type)               { throw RepMismatch(); }
  else                                   { z.type=x.type; }

  z.allocate();
  if (z.type!=Polynomial)
    {
      z.a.resize(z.num_slots());
      for (unsigned int i=0; i<z.a.size(); i++)
        { z.a[i]= (x.a[i] - y.a[i]); }
    }
  if (z.type!=Evaluation)
    { for (unsigned int i=0; i<z.b.size(); i++)
        {
          z.b[i]=x.b[i];
          z.b[i]-=y.b[i];
          if (z.b[i]<0)
            { z.b[i]+=(*z.field_data).get_prime(); }
        }
    }
}





template<>
void sub(Plaintext<Gf2nShort,P2Data,int>& z,const Plaintext<Gf2nShort,P2Data,int>& x,
                                       const Plaintext<Gf2nShort,P2Data,int>& y)
{
  if (z.field_data!=x.field_data)  { throw FieldMismatch(); }
  if (z.field_data!=y.field_data)  { throw FieldMismatch(); }

  if (x.type==Both && y.type!=Both)      { z.type=y.type; }
  else if (y.type==Both && x.type!=Both) { z.type=x.type; }
  else if (x.type!=y.type)               { throw RepMismatch(); }
  else                                   { z.type=x.type; }

  z.allocate();
  if (z.type!=Polynomial)
    {
      z.a.resize(z.num_slots());
      for (unsigned int i=0; i<z.a.size(); i++)
        { z.a[i].sub(x.a[i],y.a[i]); }
    }
  if (z.type!=Evaluation)
    { for (unsigned int i=0; i<z.b.size(); i++)
        {
          z.b[i]=x.b[i] ^ y.b[i];
        }
    }
}



template<class T,class FD,class S>
void mul(Plaintext<T,FD,S>& z,const Plaintext<T,FD,S>& x,const Plaintext<T,FD,S>& y)
{
  if (z.field_data!=x.field_data)  { throw FieldMismatch(); }
  if (z.field_data!=y.field_data)  { throw FieldMismatch(); }

  if (y.type==Polynomial) { throw NotImplemented(); }
  if (x.type==Polynomial) { throw NotImplemented(); }
  z.type=Evaluation;

  z.allocate();
  for (unsigned int i=0; i<z.a.size(); i++)
    { z.a[i] = (x.a[i] * y.a[i]); }
}


template<>
void Plaintext<Gfp,FftData,Bigint>::negate()
{
  if (type!=Polynomial)
    {
      a.resize(num_slots());
      for (unsigned int i=0; i<a.size(); i++)
        { a[i].negate(); }
    }
  if (type!=Evaluation)
    { for (unsigned int i=0; i<b.size(); i++)
        { if (b[i]!=0)
            {
              b[i]-=(*field_data).get_prime();
              b[i].negate();
            }
	}
    }
}



template<>
void Plaintext<Gf2nShort,P2Data,int>::negate()
{
  return;
}



template<class T, class FD, class _>
AddableVector<typename FD::poly_type> Plaintext<T, FD, _>::mult_by_x_i(int i,
    const FHE_PK& pk) const
{
  return AddableVector<S>(get_polynomial()).mult_by_x_i(i, pk);
}



template<class T,class FD,class S>
bool Plaintext<T,FD,S>::equals(const Plaintext& x) const
{
  if (field_data!=x.field_data) { return false; }
  if (type!=x.type)
    { if (type==Evaluation) { x.from_polynomial(); }
      else                  { from_polynomial(); }
    }

  if (type!=Polynomial and x.type!=Polynomial)
    {
      a.resize(num_slots());
      for (unsigned int i=0; i<a.size(); i++)
       { if (!(a[i] == x.a[i])) { return false; } }
    }
  else
    { for (unsigned int i=0; i<b.size(); i++)
       { if (b[i]!=x.b[i]) { return false; } }
    }
  return true;
}

template<>
bool Plaintext<Gfp, FftData, Bigint>::is_diagonal() const
{
  if (type != Evaluation)
    {
      for (size_t i = 1; i < b.size(); i++)
        if (b[i] != 0)
          return false;
    }

  if (type != Polynomial)
    {
      auto first = a[0];
      for (auto& x : a)
        if (x != first)
          return false;
    }

  return true;
}

template<>
bool Plaintext<Gf2nShort, P2Data, int>::is_diagonal() const
{
  if (type == Polynomial)
    from_polynomial();

  auto first = a[0];
  for (auto& x : a)
    if (x != first)
      return false;

  return true;
}



template <class T,class FD,class S>
void Plaintext<T,FD,S>::pack(OctetStream& o) const
{
  to_polynomial();
  o.store(b);
}

template <class T,class FD,class S>
void Plaintext<T,FD,S>::unpack(OctetStream& o)
{
  o.get(b);
  auto size = b.size();
  allocate(Polynomial);
  if (size != b.size() and size != 0)
    throw length_error("unexpected length received");
}



template <>
size_t Plaintext<Gfp, FftData, Bigint>::report_size(ReportType type)
{
  size_t res = 0;
  if (type != MINIMAL)
    res += sizeof(Gfp) * a.capacity();
  for (unsigned int i = 0; i < b.size(); i++)
    res += b[i].report_size(type);
  return res;
}


template <>
size_t Plaintext<Gf2nShort, P2Data, int>::report_size(ReportType type)
{
  size_t res = 0;
  if (type != MINIMAL)
    res += sizeof(Gf2nShort) * a.capacity();
  res += sizeof(int) * b.size();
  return res;
}


template<class T, class FD, class S>
void Plaintext<T, FD, S>::print_evaluation(int n_elements, string desc) const
{
  cout << desc;
  for (int i = 0; i < n_elements; i++)
    cout << " " << element(i);
  cout << endl;
}



template class Plaintext<Gfp,FftData,Bigint>;

template void mul(Plaintext<Gfp,FftData,Bigint>& z,const Plaintext<Gfp,FftData,Bigint>& x,const Plaintext<Gfp,FftData,Bigint>& y);



template class Plaintext<Gf2nShort,P2Data,int>;

template void mul(Plaintext<Gf2nShort,P2Data,int>& z,const Plaintext<Gf2nShort,P2Data,int>& x,const Plaintext<Gf2nShort,P2Data,int>& y);

