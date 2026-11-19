/*
 * support/mmo.h
 *
 */

#pragma once

#include "support/aes.h"
#include "garbled/bmr/key.h"

// Matyas-Meyer-Oseas hashing
class MMO
{
#ifdef GFP_MOD_SZ
    static const int N_KEYS = GFP_MOD_SZ > 8 ? GFP_MOD_SZ : 8;
#else
    static const int N_KEYS = 8;
#endif

    Octet IV[N_KEYS][176]  __attribute__((aligned (16)));

    template<int N>
    static void encrypt_and_xor(__m128i* output, const __m128i* input,
            const Octet* key);
    template<int N>
    static void encrypt_and_xor(void* output, const void* input,
            const Octet* key);
    template<int N>
    static void encrypt_and_xor(void* output, const void* input,
            const Octet* key, const int* indices);

public:
    MMO() { zero_iv(); }
    void zero_iv();
    void set_iv(int i, Octet key[AES_BLK_SIZE]);
    template <class T>
    void hash_one_block(void* output, const void* input) { hashBlocks<T, 1>((T*)output, input); }
    template <int N, int N_BYTES>
    void hashBlocks(void* output, const void* input, size_t alloc_size);
    template <class T, int N>
    void hashBlocks(void* output, const void* input);
    template <class T>
    void hash_eight_blocks(T* output, const void* input);
    template <class T, int N_BYTES>
    void hash_eight_blocks(T* output, const void* input);
    template <int X, int L>
    void hash_eight_blocks(Gfp_<X, L>* output, const void* input);
    template <int X, int L>
    void hash_eight_blocks(Gfpvar_<X, L>* output, const void* input);
    template <class T>
    void output_one_block(Octet* output);
    Key hash(const Key& input);
    template <int N>
    void hash(Key* output, const Key* input);
};

template<int N>
inline void MMO::encrypt_and_xor(__m128i* out, const __m128i* in, const Octet* key)
{
    ecb_aes_128_encrypt<N>(out, in, key);
    for (int i = 0; i < N; i++)
        out[i] = _mm_xor_si128(out[i], in[i]);
}

template<int N>
inline void MMO::encrypt_and_xor(void* output, const void* input, const Octet* key)
{
    __m128i in[N], out[N];
    avx_memcpy(in, input, sizeof(in));
    encrypt_and_xor<N>(out, in, key);
    avx_memcpy(output, out, sizeof(out));
}

inline Key MMO::hash(const Key& input)
{
    Key res;
    encrypt_and_xor<1>(&res.r, &input.r, IV[0]);
    return res;
}

template <int N>
inline void MMO::hash(Key* output, const Key* input)
{
    encrypt_and_xor<N>(&output->r, &input->r, IV[0]);
}
