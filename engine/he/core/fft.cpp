
#include "he/core/fft.h"
#include "numeric/zp_data.h"
#include "vm/base_machine.h"
#include "support/code_locations.h"

#include "numeric/modp.hpp"


/* Computes the FFT via Horner's Rule
   theta is assumed to be an Nth root of unity
*/
void naive_fft(vector<modp>& ans,vector<modp>& a,int N,const modp& theta,const ZpData& pr_d)
{
  int i,j;
  modp theta_pow;
  assignOne(theta_pow,pr_d);
  for (i=0; i<N; i++)
    { ans[i]=a[N-1];
      for (j=N-2; j>=0; j--)
	{ Mul(ans[i],ans[i],theta_pow,pr_d);
          Add(ans[i],ans[i],a[j],pr_d);
        }
      Mul(theta_pow,theta_pow,theta,pr_d); 
    }
}


void FFT(vector<modp>& a,int N,const modp& theta,const ZpData& pr_d)
{
  CODE_LOCATION_NO_SCOPE

  if (N==1) { return; }

  if (N<5)
    { vector<modp> b(N);
      naive_fft(b,a,N,theta,pr_d);
      a=b;
      return;
    }

  vector<modp> a0(N/2),a1(N/2);
  int i;
  for (i=0; i<N/2; i++)
    { a0[i]=a[2*i];
      a1[i]=a[2*i+1];
    }
  modp theta2,w,t;
  Sqr(theta2,theta,pr_d);
  FFT(a0,N/2,theta2,pr_d);
  FFT(a1,N/2,theta2,pr_d);
  assignOne(w,pr_d);
  for (i=0; i<N/2; i++)
    { Mul(t,w,a1[i],pr_d);
      Add(a[i],a0[i],t,pr_d);
      Sub(a[i+N/2],a0[i],t,pr_d);
      Mul(w,w,theta,pr_d);
    }
}


/*
 * Standard FFT for n a power of two, root a n-th primitive root of unity.
 */
template<class T,class P>
void fft_iter(vector<T>& ioput, int n, const T& root, const P& pr_d)
{
    int i, j, m;
    T t;
    
    // Bit-reversal of input
    for( i = j = 0; i < n; ++i )
    {
        if( j >= i )
        {
            t = ioput[i];
            ioput[i] = ioput[j];
            ioput[j] = t;
        }
        m = n / 2;
        
        while( (m >= 1) && (j >= m) )
        {
            j -= m;
            m /= 2;
        }
        j += m;
    }
    T u, alpha, alpha2;
    
    m = 0; j = 0; i = 0;
    // Do the transform
    for (int s = 1; s < n; s = 2*s)
    {
        m = 2*s;
        Power(alpha, root, n/m, pr_d);
        assignOne(alpha2,pr_d);
        for (int j = 0; j < m/2; ++j)
        {
            //root = root_table[j*n/m];
            for (int k = j; k < n; k += m)
            {
                Mul(t, alpha2, ioput[k + m/2], pr_d);
                u = ioput[k];
                Add(ioput[k], u, t, pr_d);
                Sub(ioput[k + m/2], u, t, pr_d);
            }
            Mul(alpha2, alpha2, alpha, pr_d);
        }
    }
}



/*
 * FFT modulo x^n + 1.
 *
 * n must be a power of two, root a 2n-th primitive root of unity.
 */
void fft_iter2(vector<modp>& ioput, int n, const modp& root, const ZpData& pr_d)
{
    fft_iter(ioput, n, root, pr_d, false);
}

void fft_iter2(vector<modp>& ioput, int n, const vector<modp>& roots,
        const ZpData& pr_d)
{
    fft_iter(ioput, n, roots, pr_d, false);
}

void fft_iter(vector<modp>& ioput, int n, const modp& root, const ZpData& pr_d,
        bool start_with_one)
{
    vector<modp> roots(n + 1);
    assignOne(roots[0], pr_d);
    for (int i = 1; i < n + 1; i++)
        Mul(roots[i], roots[i - 1], root, pr_d);
    fft_iter(ioput, n, roots, pr_d, start_with_one);
}

void fft_iter(vector<modp>& ioput, int n, const vector<modp>& roots,
        const ZpData& pr_d, bool start_with_one)
{
    CODE_LOCATION_NO_SCOPE

    assert(roots.size() > size_t(n));

    int i, j, m;
    
    // Bit-reversal of input
    for( i = j = 0; i < n; ++i )
    {
        if( j >= i )
        {
            swap(ioput[i], ioput[j]);
        }
        m = n / 2;
        
        while( (m >= 1) && (j >= m) )
        {
            j -= m;
            m /= 2;
        }
        j += m;
    }
    m = 0; j = 0; i = 0;
    // Do the transform
    vector<modp> alpha2;
    alpha2.reserve(n / 2);
    for (int s = 1; s < n; s = 2*s)
    {
        m = 2*s;

        alpha2.clear();
        if (start_with_one)
        {
            for (int j = 0; j < m / 2; j++)
                alpha2.push_back(roots[j * n / m]);
        }
        else
        {
            for (int j = 0; j < m / 2; j++)
                alpha2.push_back(roots.at((j * 2 + 1) * (n / m)));
        }

        if (BaseMachine::thread_num == 0 and BaseMachine::has_singleton())
        {
            auto& queues = BaseMachine::s().queues;
            FftJob job(ioput, alpha2, m, pr_d);
            int start = queues.distribute(job, n / 2);
            for (int i = start; i < n / 2; i++)
                fft_iter2_body(ioput, alpha2, i, m, pr_d);
            if (start > 0)
                queues.wrap_up(job);
        }
        else
            for (int i = 0; i < n / 2; i++)
                fft_iter2_body(ioput, alpha2, i, m, pr_d);
    }
}


/* This does FFT for X^N+1,
   Input and output is an array of size N (shared)
   alpha is assumed to be a generator of the N'th roots of unity mod p
   Starts at w=alpha and updates by alpha^2
*/
void FFT2(vector<modp>& a, int N, const modp& alpha, const ZpData& pr_d)
{
  int i;
  if (N==1) { return; }

  vector<modp> a0(N/2),a1(N/2);
  for (i=0; i<N/2; i++)
    { a0[i]=a[2*i];
      a1[i]=a[2*i+1];
    }

  modp w,alpha2,temp;
  Sqr(alpha2,alpha,pr_d);
  FFT2(a0,N/2,alpha2,pr_d);    FFT2(a1,N/2,alpha2,pr_d);

  w=alpha;
  for (i=0; i<N/2; i++)
    { Mul(temp,w,a1[i],pr_d);
      Add(a[i],a0[i],temp,pr_d);
      Sub(a[i+N/2],a0[i],temp,pr_d);
      Mul(w,w,alpha2,pr_d);
    }
}


void fft_non_power_of_two(vector<modp>& res, const vector<modp>& input, const FftData& FFTD)
{
    vector<modp> tmp(FFTD.m());
    BFFT(tmp, input, FFTD);
    for (int i = 0; i < (FFTD).phi_m(); i++)
        res[i] = tmp[(FFTD).p(i)];
}

void BFFT(vector<modp>& ans,const vector<modp>& a,const FftData& FFTD,bool forward)
{
  int k2=FFTD.twop,n=FFTD.m();
  if (k2<0) { k2=-k2; }
  int r=0;
  if (forward==false) { r=1; }

  if (FFTD.twop>0)
     { vector<modp> x(k2);
       for (unsigned int i=0; i<a.size(); i++)
         { Mul(x[i],FFTD.powers[r][i],a[i],FFTD.get_pr_d()); }
       for (int i=a.size(); i<k2; i++)
         { assignZero(x[i],FFTD.get_pr_d()); }
       fft_iter(x,k2,FFTD.two_root[0],FFTD.get_pr_d());
     
       for (int i=0; i<k2; i++)
          { Mul(x[i],x[i],FFTD.b[r][i],FFTD.get_pr_d()); }
     
       fft_iter(x,k2,FFTD.two_root[1],FFTD.get_pr_d());
       
       for (int i=0; i<n; i++)
         { Mul(ans[i],x[i+n-1],FFTD.powers_i[r][i],FFTD.get_pr_d()); }
     }
  else
     { throw CrashRequested(); }
}
