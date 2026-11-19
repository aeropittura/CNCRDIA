#pragma once

/* FFT routine (mod p) for when N is a power of 2
     - We do not check that N is a power of 2 though
   This is the "classic" FFT algorithm
   theta is assumed to be an Nth root of unity

   Not tried to optimize memory thrashing though
*/

#include <vector>

#include "he/core/fft_data.h"

/* This is the basic 2^n - FFT routine, it works in an recursive 
 * manner. The fft_iter routine below does the same thing but in 
 * an iterative manner, and is therefore more efficient.
 */
void FFT(vector<modp>& a,int N,const modp& theta,const ZpData& pr_d);

/* This uses a different root of unity and is used for going
 * forward when doing 2^n FFT's. This avoids the need for padding
 * out to a vector of size 2*n. Going backwards you still need to
 * use FFT, but some other post-processing. Again the iterative
 * version is given by fft_iter2 below, and this is usually faster
 */
void FFT2(vector<modp>& a,int N,const modp& theta,const ZpData& pr_d);

template <class T,class P>
void fft_iter(vector<T>& a,int N,const T& theta,const P& pr_d);

void fft_iter(vector<modp>& a, int N, const modp& theta, const ZpData& pr_d,
        bool start_with_one = true);
void fft_iter2(vector<modp>& a,int N,const modp& theta,const ZpData& pr_d);

// variants with precomputed roots

void fft_iter(vector<modp>& a, int N, const vector<modp>& theta,
        const ZpData& pr_d, bool start_with_one = true);
void fft_iter2(vector<modp>& a, int N, const vector<modp>& theta,
        const ZpData& pr_d);

inline void fft_iter2_body(vector<modp>& ioput, const vector<modp>& alpha2, int i,
        int m, const ZpData& pr_d)
{
    int j = i % (m / 2);
    int kk = i / (m / 2);
    int k = j + kk * m;
    modp t, u;
    Mul(t, alpha2[j], ioput[k + m / 2], pr_d);
    u = ioput[k];
    Add(ioput[k], u, t, pr_d);
    Sub(ioput[k + m / 2], u, t, pr_d);
}

/* BFFT perform FFT and inverse FFT  mod pr_d for non power of two cyclotomics.  
 * The modulus in pr_d (contained in FftData) must be set up
 * correctly in particular
 *     m divides pr-1     <- Otherwise FFT not defined over base field
 *     2^k divides pr-1   <- Where 2^k is the smallest power of two
 *                           greater than m. This means we can do the
 *                           trick of extension and the convolution using
 *                           the 2-power FFT to do the main work
 * forward=true  => FFT
 * forward=false => inverse FFT
 *
 * a on input is assumed to have size less than FFTD.n (it is then zero padded)
 * ans is assumed to have size FFTD.n
 */                           

void BFFT(vector<modp>& ans,const vector<modp>& a,const FftData& FFTD,bool forward=true);

void fft_non_power_of_two(vector<modp>& res, const vector<modp>& input,
        const FftData& FFTD);

/* Computes the FFT via Horner's Rule
   theta is assumed to be an Nth root of unity
   This works for any value of N
*/
void naive_fft(vector<modp>& ans,vector<modp>& a,int N,const modp& theta,const ZpData& pr_d);
