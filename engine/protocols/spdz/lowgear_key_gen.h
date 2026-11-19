/*
 * protocols/spdz/lowgear_key_gen.h
 *
 */

#pragma once

#include "protocols/common/share_vector.h"
#include "he/offline/pairwise_machine.h"
#include "protocols/spdz/mascot_prep.h"
#include "vm/processor.h"
#include "boolean/tinier_secret.h"
#include "numeric/gfp.h"
#include "numeric/gfpvar.h"

/**
 * Homomorphic key component generation (modulo a prime) using MASCOT
 */
template<int X, int L>
class KeyGeneratorProtocol
{
public:
    typedef Share<Gfpvar_<X, MAX_MOD_SZ>> ShareType;
    typedef typename ShareType::OpenType OpenType;
    typedef ShareVector<ShareType> VectorType;

    int backup_batch_size;

protected:
    Player& P;
    const FheParameters& params;
    const FftData& fftd;

    SeededPrng G;
    DataPositions usage;

    ShareType get_bit()
    {
        return prep->get_bit();
    }

public:
    Preprocessing<ShareType>* prep;
    MacCheck_<ShareType>* MC;
    SubProcessor<ShareType>* proc;

    KeyGeneratorProtocol(Player& P, const FheParameters& params, int level = 0);
    ~KeyGeneratorProtocol();

    void input(vector<VectorType>& shares, const RqElement& secret);
    template<class T>
    void binomial(VectorType& shares, T& prep);
    template<class T>
    void secret_key(VectorType& shares, T& prep);
    VectorType schur_product(const VectorType& x, const VectorType& y);
    void output_to(int player, vector<OpenType>& opened,
            vector<ShareType>& shares);
};

/**
 * Semi-homomorphic key generation using MASCOT
 */
template<int L>
class LowGearKeyGenerator : public KeyGeneratorProtocol<1, L>
{
    typedef KeyGeneratorProtocol<1, L> Super;

    typedef typename Super::ShareType ShareType;
    typedef typename Super::OpenType OpenType;
    typedef typename Super::VectorType VectorType;

    Player& P;
    PairwiseMachine& machine;

    void generate_keys(FheParameters& params);

public:
    LowGearKeyGenerator(Player& P, PairwiseMachine& machine, FheParameters& params);

    template<class FD>
    void run(PairwiseSetup<FD>& setup);
};
