#pragma once

#include "support/octet_stream.h"
#include "support/hash.h"
#include "support/aes.h"
#include "support/avx_memcpy.h"
#include "support/time_func.h"
#include "net/data.h"

#include <gmp.h>

#define USE_AES

#ifndef USE_AES
  #define PIPELINES   1
  #define SEED_SIZE   randombytes_SEEDBYTES
  #define CALL_SIZE   480
#else
#if defined(__AES__) || !defined(__x86_64__)
  #define PIPELINES   8
#else
  #define PIPELINES   1
#endif
  #define SEED_SIZE   AES_BLK_SIZE
  #define CALL_SIZE   (PIPELINES * AES_BLK_SIZE)
#endif

class Player;
class PlayerBase;
class BitVector;

/* This basically defines a randomness expander, if using
 * as a real PRG on an input stream you should first collapse
 * the input stream down to a SEED, say via CBC-MAC (under 0 key)
 * or via a hash
 */

// __attribute__ is needed to get the sse instructions to avoid
//  seg faulting.

/**
 * Pseudo-random number generator. This uses counter-mode AES by default,
 * which can be changed libsodium's expansion by undefining ``USE_AES``.
 */
class PRNG
{
   static const int N_CACHE = 10;
   static const int RAND_SIZE = N_CACHE * CALL_SIZE;

   Octet seed[SEED_SIZE]; 
   Octet state[RAND_SIZE] __attribute__((aligned (16)));
   Octet random[RAND_SIZE] __attribute__((aligned (16)));

   #ifdef USE_AES
#if defined(__AES__) || !defined(__x86_64__)
     bool use_c;
#else
     const static bool use_c = true;
#endif

     // Two types of key schedule for the different implementations 
     // of AES
     uint  key_schedule_c[44];
     Octet KeySchedule[176]  __attribute__((aligned (16)));
   #endif

   int cnt;    // How many bytes of the current random value have been used

   int n_cached_bits;
   word cached_bits;

   bool initialized;

   template<bool SMALL = false>
   void hash(); // Hashes state to random and sets cnt=0

   public:

   Timer timer;

   /// Construction without initialization. Usage without initialization will fail.
   PRNG();
   /// Initialize with ``SEED_SIZE`` bytes from buffer.
   PRNG(OctetStream& seed);
   /// Initialize with ``SEED_SIZE`` bytes from buffer.
   PRNG(const string& seed);

   // For debugging
   void print_state() const;

   /// Initialize from local randomness.
   void re_seed();

   // Agree securely on seed
   void seed_globally(const PlayerBase& P,
           const vector<bool>& parties = { });

   /**
    * Coordinate random seed
    * @param P communication instances
    * @param secure seeding prevents tampering at higher cost
    */
   void seed_globally(const Player& P, bool secure = true);

   /// Initialize with ``SEED_SIZE`` bytes from pointer.
   void SetSeed(const unsigned char*);
   /// Initialize with seed from another instance.
   void SetSeed(PRNG& G);
   /// Initialize with seed from any serializable
   template<class T>
   void set_seed_from(const T& seed);
   void initialize_seed();
   
   template<bool SMALL = false>
   void next();

   bool is_initialized();

   /// Random bit
   bool get_bit();
   /// Random bytes
   unsigned char get_uchar();
   /// Random 32-bit integer
   unsigned int get_uint();
   /// Random 32-bit integer between 0 and ``upper``
   unsigned int get_uint(int upper);
   unsigned int get_uint_lemire(unsigned int upper);

   /* Random integer of any length
    * @res result
    * @n_bits number of bits
    * @positive positive result (random sign otherwise)
    */
   void get(Bigint& res, int n_bits, bool positive = true);

   /**
    * Random integer in ``[0, B-1]``
    * @param res result
    * @param B bound
    * @param positive positive result (random sign otherwise)
    */
   template<class T>
   void random_bnd(T& res, const Bigint& B, bool positive=true);
   template<int N_BYTES>
   void random_bnd(mp_limb_t* res, const mp_limb_t* B, mp_limb_t mask = -1);
   void random_bnd(mp_limb_t* res, const mp_limb_t* B, size_t n_bytes, mp_limb_t mask = -1);

   /// Random 64-bit integer
   word get_word()
     {
       word a;
       get_octets<sizeof(a)>((Octet*)&a);
       return le64toh(a);
     }

   /// Random 128-bit integer
   __m128i get_doubleword();

   /*
    * Fill buffer with random data
    * @param ans result
    * @param len byte length
    */
   void get_octet_stream(OctetStream& ans,int len);

   /**
    * Fill array with random data
    * @param ans result
    * @param len byte length
    */
   void get_octets(Octet* ans, int len);

   // non-inlined version
   void get_octets_call(Octet* ans, int len);

   /**
    * Fill array with random data (compile-time length)
    * @param ans result
    */
   template <int L>
   void get_octets(Octet* ans);

   const Octet* get_seed() const
     { return seed; }

   /// Random instance of any supported class
   template<class T>
   T get()
     { T res; res.randomize(*this); return res; }

   template<class T>
   void random_permutation(size_t n, T& perm);
};

/// Randomly seeded pseudo-random number generator
class SeededPrng : public PRNG
{
public:
  SeededPrng()
  {
    re_seed();
  }
};

/// Coordinated pseudo-random number with secure seeding
class GlobalPrng : public PRNG
{
public:
  GlobalPrng(const PlayerBase& P)
  {
    seed_globally(P);
  }
};

template<class T>
class ElementPRNG : public PRNG
{
public:
  T get()
    {
      return PRNG::get<T>();
    }
};

template<class T>
void PRNG::set_seed_from(const T& seed)
{
  OctetStream os;
  seed.pack(os);
  assert(os.get_length() >= SEED_SIZE);
  SetSeed(os.get_data());
}

inline bool PRNG::get_bit()
{
  if (n_cached_bits == 0)
    {
      cached_bits = get_word();
      n_cached_bits = 64;
    }
  n_cached_bits--;
  return (cached_bits >> n_cached_bits) & 1;
}

inline unsigned char PRNG::get_uchar()
{
  if (cnt>=RAND_SIZE) { next(); }
  unsigned char ans=random[cnt];
  cnt++;
  // print_state(); cout << " UCHA " << (int) ans << endl;
  return ans;
}

inline unsigned int PRNG::get_uint()
{
  // We need four bytes of randomness
  if (cnt>RAND_SIZE-4) { next(); }
  unsigned int ans = 0;
  memcpy(&ans, random + cnt, 4);
  cnt += 4;
  // print_state(); cout << " UINT " << ans << endl;
  return ans;
}


inline __m128i PRNG::get_doubleword()
{
    if (cnt > RAND_SIZE - 16)
        next();
    __m128i ans = _mm_loadu_si128((__m128i*)&random[cnt]);
    cnt += 16;
    return ans;
}

inline void PRNG::get_octets(Octet* ans,int len)
{
  int pos=0;
  while (len)
    {
      if (cnt==RAND_SIZE)
        next();
      int step=min(len,RAND_SIZE-cnt);
      memcpy(ans+pos,random+cnt,step);
      pos+=step;
      len-=step;
      cnt+=step;
    }
}

template<int L>
inline void PRNG::get_octets(Octet* ans)
{
   if (L <= RAND_SIZE - cnt)
   {
     avx_memcpy<L>(ans, random + cnt);
     cnt += L;
   }
   else
     get_octets_call(ans, L);
}

template<int N_BYTES>
inline void PRNG::random_bnd(mp_limb_t* res, const mp_limb_t* B, mp_limb_t mask)
{
  size_t n_limbs = (N_BYTES + sizeof(mp_limb_t) - 1) / sizeof(mp_limb_t);
  do
    {
      get_octets<N_BYTES>((Octet*) res);
      res[n_limbs - 1] &= mask;
    }
  while (mpn_cmp(res, B, n_limbs) >= 0);
}

template<>
inline Octet PRNG::get()
{
  return get_uchar();
}

template<>
inline word PRNG::get()
{
  return get_word();
}

// https://arxiv.org/abs/1805.10941
inline unsigned int PRNG::get_uint_lemire(unsigned int upper)
{
  auto s = upper;
  uint64_t x = get_uint();
  uint64_t m = x * s;
  uint64_t two_l = uint64_t(1) << 32;
  uint64_t l = m % two_l;
  if (l < s)
    {
      uint64_t t = (two_l - s) % s;
      while (l < t)
        {
          x = get_uint();
          m = x * s;
          l = m % two_l;
        }
    }
  return m >> 32;
}

template<class T>
void PRNG::random_permutation(size_t n, T& perm)
{
  perm.reserve(n);
  for (size_t j = 0; j < n; j++)
    perm.push_back(j);
  for (size_t j = 0; j < n; j++)
    {
      int k = get_uint_lemire(n - j);
      swap(perm[j], perm[k + j]);
    }
}
