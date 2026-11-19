/*
 * he/offline/sacrificing.cpp
 *
 */

#include "he/core/p2_data.h"

#include "he/offline/sacrificing.h"
#include "he/offline/producer.h"

#include "support/subroutines.h"

#include "protocols/common/mac_key.hpp"

// The number of sacrifices to amortize at one time
#define amortize 512


template <class T>
void triple_checking(const Player& P, MacCheck<T>& MC, int nm,
    int output_thread, TripleSacriFactory< Share<T> >& factory, bool write_output,
    bool clear, string dir)
{
  CODE_LOCATION
  check_field_size<T>();

  ofstream outf;
  if (write_output)
    open_prep_file<T>(outf, "Triples", P.my_num(), output_thread, false,
        clear, dir);

  T te,t;
  create_random(t,P);
  vector<Share<T> > sh_po(2*amortize),sh_tau(amortize);
  vector<T> PO(2*amortize),Tau(amortize);
  vector<Share<T> > a1(amortize),b1(amortize),c1(amortize);
  vector<Share<T> > a2(amortize),b2(amortize),c2(amortize);
  Share<T> temp;

  // Triple checking
  int left_todo=nm; 
  while (left_todo>0)
    { int this_loop=amortize;
      if (this_loop>left_todo)
        { this_loop=left_todo;
          PO.resize(2*this_loop);
          sh_po.resize(2*this_loop);
          Tau.resize(this_loop);
          sh_tau.resize(this_loop);
        }
     
      for (int i=0; i<this_loop; i++)
        {
          factory.get(a1[i], b1[i], c1[i]);
          factory.get(a2[i], b2[i], c2[i]);
          sh_po[2*i].mul(a1[i],t);
          sh_po[2*i].sub(sh_po[2*i],a2[i]);
          sh_po[2*i+1].sub(b1[i],b2[i]);
	}
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      for (int i=0; i<this_loop; i++)
        { sh_tau[i].mul(c1[i],t);
          sh_tau[i].sub(sh_tau[i],c2[i]);
          temp.mul(a2[i],PO[2*i+1]);
          sh_tau[i].sub(sh_tau[i],temp);
          temp.mul(b2[i],PO[2*i]);
          sh_tau[i].sub(sh_tau[i],temp);
          te = (PO[2*i] * PO[2*i+1]);
          sh_tau[i].sub(sh_tau[i], Share<T>::constant(te,P.my_num(),MC.get_alphai()));
        }
      MC.POpen_Begin(Tau,sh_tau,P);
      MC.POpen_End(Tau,sh_tau,P);

      for (int i=0; i<this_loop; i++)
        { if (!Tau[i].is_zero())
	    {
              MC.POpen(PO, {a1[i], b1[i], c1[i], a2[i], b2[i], c2[i]}, P);
              for (int j = 0; j < 2; j++)
                {
                  auto prod = PO[3 * j + 1] * PO[3 * j];
                  if (PO[3 * j + 2] != prod)
                    cout << PO[3 * j + 2] << " != " << prod << " = "
                        << PO[3 * j + 1] << " * " << PO[3 * j] << endl;
                }
              throw OfflineCheckError("Multiplication Triples");
	    }
          if (write_output)
            {
              a1[i].output(outf,false);
              b1[i].output(outf,false);
              c1[i].output(outf,false);
            }
          else
              factory.triples.push_back({{a1[i], b1[i], c1[i]}});
        }

      left_todo-=this_loop;
    }

  if (write_output)
    outf.close();
}


template <class T>
void inverse_checking(const Player& P, MacCheck<T>& MC, int nr,
    int output_thread, TripleSacriFactory<Share<T> >& triple_factory,
    TupleSacriFactory<Share<T> >& inverse_factor, bool write_output,
    bool clear, string dir)
{
  CODE_LOCATION
  ofstream outf_inverse;
  if (write_output)
    open_prep_file<T>(outf_inverse, "Inverses", P.my_num(), output_thread, false,
        clear, dir);

  T te,t;
  create_random(t,P);
  vector<Share<T> > sh_po(2*amortize),sh_tau(amortize);
  vector<T> PO(2*amortize),Tau(amortize);
  vector<Share<T> > a1(amortize),b1(amortize),c1(amortize);
  vector<Share<T> > a2(amortize),b2(amortize),c2(amortize);
  Share<T> temp;

  // Inverse checking
  int left_todo=nr;
  while (left_todo>0)
    { int this_loop=amortize;
      if (this_loop>left_todo)
        { this_loop=left_todo;
          PO.resize(2*this_loop);
          sh_po.resize(2*this_loop);
          Tau.resize(this_loop);
          sh_tau.resize(this_loop);
        }

      for (int i=0; i<this_loop; i++)
        {
          inverse_factor.get(a1[i], b1[i]);
          triple_factory.get(a2[i], b2[i], c2[i]);
          sh_po[2*i].mul(a1[i],t);
          sh_po[2*i].sub(sh_po[2*i],a2[i]);
          sh_po[2*i+1].sub(b1[i],b2[i]);
        }
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      auto t_shared = Share<T>::constant(t, P.my_num(), MC.get_alphai());

      for (int i=0; i<this_loop; i++)
        {
          sh_tau[i].sub(c2[i],t_shared);
          temp.mul(a2[i],PO[2*i+1]);
          sh_tau[i].add(sh_tau[i],temp);
          temp.mul(b1[i],PO[2*i]);
          sh_tau[i].add(sh_tau[i],temp);
        }
      MC.POpen_Begin(Tau,sh_tau,P);
      MC.POpen_End(Tau,sh_tau,P);

      for (int i=0; i<this_loop; i++)
        { if (!Tau[i].is_zero())
            { throw OfflineCheckError("Inverses"); }
          if (write_output)
            {
              a1[i].output(outf_inverse,false);
              b1[i].output(outf_inverse,false);
            }
        }

      left_todo-=this_loop;
    }

  if (write_output)
    {
      outf_inverse.close();
    }
}


template <class T>
void square_checking(const Player& P, MacCheck<T>& MC, int ns,
        int output_thread, TupleSacriFactory<Share<T> >& square_factory,
        bool write_output, bool clear, string dir)
{
  CODE_LOCATION
  ofstream outf_s, outf_b;
  if (write_output)
  {
    open_prep_file<T>(outf_s, "Squares", P.my_num(), output_thread, false, clear, dir);
  }

  T te,t,t2;
  create_random(t,P);
  t2 = t * t;
  vector<Share<T> > sh_po(amortize);
  vector<T> PO(amortize);
  vector<Share<T> > f(amortize),h(amortize),a(amortize),b(amortize);
  Share<T>  temp;

  // Do the square checking
  int left_todo=ns;
  square_factory.tuples.clear();
   while (left_todo>0)
    { int this_loop=amortize;
      if (this_loop>left_todo)
        { this_loop=left_todo;
          PO.resize(this_loop);
          sh_po.resize(this_loop);
        }

      for (int i=0; i<this_loop; i++)
        {
          square_factory.get(f[i], h[i]);
          square_factory.get(a[i], b[i]);

          sh_po[i].mul(a[i],t);
          sh_po[i].sub(sh_po[i],f[i]);
        }
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      for (int i=0; i<this_loop; i++)
        { sh_po[i].mul(b[i],t2);
          sh_po[i].sub(sh_po[i],h[i]);
          temp.mul(a[i],t);
          temp.add(temp,f[i]);
          temp.mul(temp,PO[i]);
          sh_po[i].sub(sh_po[i],temp);
        }
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      for (int i=0; i<this_loop; i++)
        { if (!PO[i].is_zero())
            { throw OfflineCheckError("Squares"); }
          if (write_output)
            {
              a[i].output(outf_s,false); b[i].output(outf_s,false);
            }
          else
              square_factory.tuples.push_back({{a[i], b[i]}});
        }
      left_todo-=this_loop;
    }
  outf_s.close(); 
}

void bit_checking(const Player& P, MacCheck<Gfp>& MC, int nb,
        int output_thread, TupleSacriFactory<Share<Gfp> >& square_factory,
        SingleSacriFactory<Share<Gfp> >& bit_factory, bool write_output,
        bool clear, string dir)
{
  CODE_LOCATION
  Gfp dummy;
  ofstream outf_b;
  if (write_output)
    open_prep_file<Gfp>(outf_b, "Bits", P.my_num(), output_thread, false, clear,
        dir);

  Gfp te,t,t2;
  create_random(t,P);
  t2 = t * t;
  vector<Share<Gfp> > sh_po(amortize);
  vector<Gfp> PO(amortize);
  vector<Share<Gfp> > f(amortize),h(amortize),a(amortize),b(amortize);
  Share<Gfp>  temp;

  // Do the bits checking
  PO.resize(amortize);
  sh_po.resize(amortize);
  int left_todo=nb;
  while (left_todo>0)
    { int this_loop=amortize;
      if (this_loop>left_todo)
	{ this_loop=left_todo;
          PO.resize(this_loop);
          sh_po.resize(this_loop);
	}

      for (int i=0; i<this_loop; i++)
        {
          square_factory.get(f[i], h[i]);
          bit_factory.get(a[i]);

          sh_po[i].mul(a[i],t);
          sh_po[i].sub(sh_po[i],f[i]);
	}
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      for (int i=0; i<this_loop; i++)
        { sh_po[i].mul(a[i],t2);
          sh_po[i].sub(sh_po[i],h[i]);
          temp.mul(a[i],t);
          temp.add(temp,f[i]);
          temp.mul(temp,PO[i]);
          sh_po[i].sub(sh_po[i],temp);
        }
      MC.POpen_Begin(PO,sh_po,P);
      MC.POpen_End(PO,sh_po,P);

      for (int i=0; i<this_loop; i++)
        { if (!PO[i].is_zero())
            { throw OfflineCheckError("Bits"); }
          if (write_output)
            a[i].output(outf_b,false);
	}

      left_todo-=this_loop;
    }
  outf_b.close();
}


template void triple_checking(const Player& P, MacCheck<Gfp>& MC, int nm,
        int output_thread, TripleSacriFactory<Share<Gfp> >& factory,
        bool write_output, bool clear, string dir);
template void triple_checking(const Player& P, MacCheck<Gf2nShort>& MC,
        int nm, int output_thread,
        TripleSacriFactory<Share<Gf2nShort> >& factory, bool write_output,
        bool clear, string dir);

template void square_checking(const Player& P, MacCheck<Gfp>& MC, int ns,
        int output_thread, TupleSacriFactory<Share<Gfp> >& square_factory,
        bool write_output, bool clear, string dir);
template void square_checking(const Player& P, MacCheck<Gf2nShort>& MC,
        int ns, int output_thread,
        TupleSacriFactory<Share<Gf2nShort> >& square_factory,
        bool write_output, bool clear, string dir);

template void inverse_checking(const Player& P, MacCheck<Gfp>& MC, int nr,
        int output_thread, TripleSacriFactory<Share<Gfp> >& triple_factory,
        TupleSacriFactory<Share<Gfp> >& inverse_factor, bool write_output,
        bool clear, string dir);
template void inverse_checking(const Player& P, MacCheck<Gf2nShort>& MC, int nr,
        int output_thread, TripleSacriFactory<Share<Gf2nShort> >& triple_factory,
        TupleSacriFactory<Share<Gf2nShort> >& inverse_factor, bool write_output,
        bool clear, string dir);
