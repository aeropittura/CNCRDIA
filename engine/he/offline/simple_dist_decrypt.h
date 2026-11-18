/*
 * he/offline/simple_dist_decrypt.h
 *
 */

#pragma once

#include "he/offline/dist_decrypt.h"
#include "he/offline/data_setup.h"

template <class FD>
class SimpleDistDecrypt : public DistDecrypt<FD>
{
public:
    SimpleDistDecrypt(const Player& P, const PartSetup<FD>& setup) :
            DistDecrypt<FD>(P, setup.sk, setup.pk, setup.FieldD) {}

    void intermediate_step();
    void reshare(Plaintext<typename FD::T, FD, typename FD::S>& m,
        const Ciphertext& cm,
        EncCommitBase<typename FD::T, FD, typename FD::S>& EC);
    Plaintext_<FD> reshare(const Ciphertext& cm);
};
