/*
 * he/offline/dist_key_gen.h
 *
 */

#pragma once

#include "he/core/random_coins.h"
#include "he/core/rq_element.h"
#include "he/core/fhe_params.h"
#include "he/core/fhe_keys.h"
#include "net/player.h"

void Encrypt_Rq_Element(Ciphertext& c, const RqElement& mess,
        const RandomCoins& rc, const FHE_PK& pk);

void run_gen_protocol(FHE_PK& pk, FHE_SK& sk, const Player& P, int num_runs,
        bool commit);


class DistKeyGenerator
{
    const FheParameters& params;

public:
    RandomCoins rc1, rc2;
    RqElement secret;
    RqElement a;
    RqElement e;
    RqElement b;
    FHE_PK pk;
    Ciphertext enc_dash;
    Ciphertext enc;
    Bigint p;

    static void fake(FHE_PK& pk, vector<FHE_SK>& sks, const Bigint& p, int nparties);

    DistKeyGenerator(const FheParameters& params, const Bigint& p);
    void gen_random_data(PRNG& G);
    DistKeyGenerator& operator+=(const DistKeyGenerator& other);
    void sum_a(const vector<RqElement>& as);
    void compute_b();
    void compute_enc_dash(const vector<RqElement>& bs);
    void compute_enc_dash();
    void compute_enc(const vector<Ciphertext>& enc_dashs);
    void compute_enc();
    void sum_enc(const vector<Ciphertext>& enc);
    void finalize(FHE_PK& pk, FHE_SK& sk);
    void check_equality(const DistKeyGenerator& other);
};
