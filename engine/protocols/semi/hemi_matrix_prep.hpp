/*
 * protocols/semi/hemi_matrix_prep.hpp
 *
 */

#pragma once

#include "protocols/semi/hemi_matrix_prep.h"
#include "protocols/common/mac_check.h"
#include "he/core/diagonalizer.h"
#include "support/bundle.h"

class CipherPlainMultJob : public ThreadJob
{
public:
    CipherPlainMultJob(vector<Ciphertext>& products,
            const vector<Ciphertext>& multiplicands,
            const vector<Plaintext_<FftData>>& multiplicands2, bool add)
    {
        type = CIPHER_PLAIN_MULT_JOB;
        output = &products;
        input = &multiplicands;
        supply = &multiplicands2;
        length = add;
    }
};

inline void cipher_plain_mult(ThreadJob job, true_type)
{
#ifdef VERBOSE_CIPHER_PLAIN_MULT
    fprintf(stderr, "multiply %d to %d in thread %lx\n", job.begin, job.end,
            pthread_self());
    fflush(stderr);
#endif
    for (int i = job.begin; i < job.end; i++)
    {
        auto prod = ((vector<Ciphertext>*) job.input)->at(i)
                * ((vector<Plaintext_<FftData>>*) job.supply)->at(i);
        auto& results = *((vector<Ciphertext>*) job.output);

        if (job.length)
            results[job.begin] += prod;
        else
            results[i] = prod;

    }
}

inline void cipher_plain_mult(ThreadJob, false_type)
{
    throw NotImplemented();
}

class MatrixRandomMultJob : public ThreadJob
{
public:
    MatrixRandomMultJob(vector<ValueMatrix<Gfpvar>>& C,
            const vector<ValueMatrix<Gfpvar>>& A,
            vector<ValueMatrix<Gfpvar>>& B,
            bool local_mult)
    {
        type = MATRX_RAND_MULT_JOB;
        output = &C;
        input = &A;
        supply = &B;
        length = local_mult;
    }
};

inline void matrix_random_mult(ThreadJob job, true_type = {})
{
    auto& C = *(vector<ValueMatrix<Gfpvar>>*) job.output;
    auto& A = *(vector<ValueMatrix<Gfpvar>>*) job.input;
    auto& B = *(vector<ValueMatrix<Gfpvar>>*) job.supply;
    SeededPrng G;

    for (int i = job.begin; i < job.end; i++)
    {
        A[i].randomize(G);
        B[i].randomize(G);
        if (job.length)
            C[i] = A[i] * B[i];
    }
}

inline void matrix_random_mult(ThreadJob, false_type)
{
    throw NotImplemented();
}

template<class T>
int HemiMatrixPrep<T>::minimum_batch()
{
    assert(prep);
    return prep->get_ftd().num_slots() / n_rows;
}

template<class T>
void HemiMatrixPrep<T>::buffer_triples()
{
    CODE_LOCATION
    assert(prep);
    auto& multipliers = prep->get_multipliers();
    auto& FTD = prep->get_ftd();
    auto& pk = prep->get_pk();
    int n_matrices = minimum_batch();
    bool verbose = OnlineOptions::singleton.has_option("verbose_he");

    if (verbose)
    {
        fprintf(stderr, "creating %d %dx%d * %dx%d triples\n", n_matrices,
                n_rows, n_inner, n_inner, n_cols);
        fflush(stderr);
    }

    RunningTimer timer;
    AddableVector<ValueMatrix<Gfpvar>> A(n_matrices, {n_rows, n_inner}),
            B(n_matrices, {n_inner, n_cols});
    SeededPrng G;
    AddableVector<ValueMatrix<Gfpvar>> C(n_matrices);
    MatrixRandomMultJob job(C, A, B, T::local_mult);

    if (BaseMachine::thread_num == 0 and BaseMachine::has_singleton())
    {
        auto& queues = BaseMachine::s().queues;
        int start = queues.distribute(job, n_matrices);
        job.begin = start;
        job.end = n_matrices;
        matrix_random_mult(job);
        if (start)
            queues.wrap_up(job);
    }
    else
    {
        job.begin = 0;
        job.end = n_matrices;
        matrix_random_mult(job);
    }

#ifdef VERBOSE_HE
    fprintf(stderr, "encrypt at %f\n", timer.elapsed());
    fflush(stderr);
#endif

    Diagonalizer diag(A, FTD, pk);

    vector<Plaintext_<FftData>> products(n_cols, FTD);
    assert(prep->proc);
    auto& P = prep->proc->P;

    vector<vector<Ciphertext>> others_ct;

    if (T::local_mult or OnlineOptions::singleton.direct)
    {
        if (verbose)
        {
            fprintf(stderr, "broadcasting %zu ciphertexts\n",
                    diag.ciphertexts.size());
            fflush(stderr);
        }

        Bundle<OctetStream> bundle(P);
        bundle.mine.store(diag.ciphertexts);
        P.unchecked_broadcast(bundle);
        for (auto& os : bundle)
        {
            others_ct.push_back({});
            os.get(others_ct.back(), Ciphertext(pk));
        }
    }
    else
    {
        if (verbose)
        {
            fprintf(stderr, "summing %zu ciphertexts\n",
                    diag.ciphertexts.size());
            fflush(stderr);
        }

        others_ct.push_back(diag.ciphertexts);
        TreeSum<Ciphertext>().run(others_ct[0], P);
    }

    for (int j = 0; j < n_cols; j++)
        for (auto m : multipliers)
        {
#ifdef VERBOSE_HE
            fprintf(stderr, "column %d with party offset %d at %f\n", j,
                    m->get_offset(), timer.elapsed());
            fflush(stderr);
#endif
            Ciphertext C(pk);
            auto& multiplicands = m->get_multiplicands(others_ct, pk);
            if (BaseMachine::thread_num == 0 and BaseMachine::has_singleton())
            {
                auto& queues = BaseMachine::s().queues;
                vector<Ciphertext> products(n_inner, pk);
                vector<Plaintext_<FftData>> multiplicands2;
                for (int i = 0; i < n_inner; i++)
                    multiplicands2.push_back(diag.get_plaintext(B, i, j));
                CipherPlainMultJob job(products, multiplicands, multiplicands2, true);
                int start = queues.distribute(job, n_inner);
#ifdef VERBOSE_HE
                fprintf(stderr, "from %d in central thread at %f\n", start, timer.elapsed());
                fflush(stderr);
#endif
                for (int i = start; i < n_inner; i++)
                    products[i] = multiplicands.at(i) * multiplicands2.at(i);
                if (start)
                    queues.wrap_up(job);
#ifdef VERBOSE_HE
                fprintf(stderr, "adding at %f\n", timer.elapsed());
                fflush(stderr);
#endif
                for (int i = 0; i < n_inner; i++)
                    C += products[i];
            }
            else
                for (int i = 0; i < n_inner; i++)
                    C += multiplicands.at(i) * diag.get_plaintext(B, i, j);

#ifdef VERBOSE_HE
            fprintf(stderr, "adding column %d with party offset %d at %f\n", j,
                    m->get_offset(), timer.elapsed());
            fflush(stderr);
#endif
            m->add(products[j], C, BOTH, n_inner);
        }

    if (T::local_mult)
        C += diag.dediag(products, n_matrices);
    else
        C = diag.dediag(products, n_matrices);

    for (int i = 0; i < n_matrices; i++)
        if (swapped)
            this->triples.push_back(
                    {{B[i].transpose(), A[i].transpose(), C[i].transpose()}});
        else
            this->triples.push_back({{A[i], B[i], C[i]}});

#ifdef VERBOSE_HE
    fprintf(stderr, "done at %f\n", timer.elapsed());
    fflush(stderr);
#endif
}
